import pysoem
import threading
import time
import struct


def pollAllAdapters():
    for adapter in pysoem.find_adapters():
        print(str(adapter.name) + " | " + str(adapter.desc))
        master = pysoem.Master()
        master.open(adapter.name)
        if master.config_init() > 0:
            for device in master.slaves:
                print(f'  Found Device {device.name}')
        else:
            print('  No device found')


class SoemTest(object):
    def __init__(self, adapter):
        self.master : pysoem.Master = pysoem.Master()
        self.master.open(adapter)
        self.master.in_op = False
        self.master.do_check_state = False
        self._proc_thread_handle = None
        self._check_thread_handle = None
        self._pd_thread_stop_event = threading.Event()
        self._ch_thread_stop_event = threading.Event()
        self._actual_wkc = 0
        # TI ICSS requires overlap
        self._is_overlapping_enabled = True 
    
    def __del__(self):
        self._pd_thread_stop_event.set()
        self._ch_thread_stop_event.set()
        if self._proc_thread_handle:
            self._proc_thread_handle.join()
        if self._check_thread_handle:
            self._check_thread_handle.join()

        self.master.state = pysoem.INIT_STATE
        self.master.write_state()
        self.master.close()

    def configure(self):
        self.master.config_init(False)
        self.master.config_dc()
        if self._is_overlapping_enabled:
            self.master.config_overlap_map()
        else:
            self.master.config_map()
        print("%d slaves found and configured."%len(self.master.slaves))
        if not self.master.state_check(pysoem.SAFEOP_STATE) == pysoem.SAFEOP_STATE:
            print("Not all slaves reached safe operational state.")
            for i in range(len(self.master.slaves)):
                if self.master.slaves[i].state != pysoem.SAFEOP_STATE:
                    print("Slave %d State=%2x StatusCode=%4x : %s"%(
                    i, self.master.slaves[i].state, self.master.slaves[i].al_status,
                    pysoem.al_status_code_to_string(self.master.slaves[i].al_status)))
        assert self.master.state_check(pysoem.SAFEOP_STATE) == pysoem.SAFEOP_STATE

    def SDO2string(self, slave : int, index : int, subidx : int, dtype : int):
        retStr = ""
        sdo = self.master.slaves[slave].sdo_read(index, subidx)
        if dtype == pysoem.ECT_BOOLEAN:
            val = struct.unpack("<?", sdo)[0]
            if (val):
                retStr += "TRUE"
            else:
                retStr += "FALSE"
        elif dtype == pysoem.ECT_INTEGER8:
            i8 = struct.unpack("<b", sdo)[0]
            retStr += "0x%2.2x (%d)"%(i8, i8)
        elif dtype == pysoem.ECT_INTEGER16:
            i16 = struct.unpack("<h", sdo)[0]
            retStr += "0x%4.4x (%d)"%(i16, i16)
        elif dtype == pysoem.ECT_INTEGER32 or dtype == pysoem.ECT_INTEGER24:
            i32 = struct.unpack("<i", sdo)[0]
            retStr += "0x%8.8x (%d)"%(i32, i32)
        elif dtype == pysoem.ECT_INTEGER64:
            i64 = struct.unpack("<q", sdo)[0]
            retStr += "0x%16.16X (%d)"%(i64, i64)
        elif dtype == pysoem.ECT_UNSIGNED8:
            u8 = struct.unpack("<B", sdo)[0]
            retStr += "0x%2.2x (%u)"%(u8, u8)
        elif dtype == pysoem.ECT_UNSIGNED16:
            u16 = struct.unpack("<H", sdo)[0]
            retStr += "0x%4.4x (%u)"%(u16, u16)
        elif dtype == pysoem.ECT_UNSIGNED32 or dtype == pysoem.ECT_UNSIGNED24:
            u32 = struct.unpack("<I", sdo)[0]
            retStr += "0x%8.8x (%u)"%(u32, u32)
        elif dtype == pysoem.ECT_UNSIGNED64:
            u64 = struct.unpack("<Q", sdo)[0]
            retStr += "0x%16.16X (%d)"%(u64, u64)
        elif dtype == pysoem.ECT_REAL32:
            sr = struct.unpack("<f", sdo)[0]
            retStr += "%f"%(sr)
        elif dtype == pysoem.ECT_REAL64:
            dr = struct.unpack("<d", sdo)[0]
            retStr += "%f"%(dr)
        elif dtype in [pysoem.ECT_BIT1, pysoem.ECT_BIT2, pysoem.ECT_BIT3, pysoem.ECT_BIT4, pysoem.ECT_BIT5, pysoem.ECT_BIT6, pysoem.ECT_BIT7, pysoem.ECT_BIT8]:
            u8 = struct.unpack("<b", sdo)[0]
            retStr += "0x%x"%(u8)
        elif dtype == pysoem.ECT_VISIBLE_STRING:
            retStr += str(sdo, "utf-8")
        elif dtype == pysoem.ECT_OCTET_STRING:
            for i in range(len(sdo)):
                retStr += "0x%2.2x "%(sdo[i])
        else:
            retStr += "Unknown type"
        return retStr

    def print_od(self):
        for cnt, ec_slave in enumerate(self.master.slaves):
            print(" CoE Object Description found, %d entries."%len(ec_slave.od))
            for obj in ec_slave.od:
                print(" Index: %4.4x Datatype: %4.4x Objectcode: %2.2x Name: %s"%(
                    obj.index, obj.data_type, obj.obj_access, obj.name))
                if len(obj.entries) == 0:
                    print("        Value: %s"%(self.SDO2string(cnt, obj.index, 0, obj.data_type)))
                for subi in range(len(obj.entries)):
                    subobj = obj.entries[subi]
                    if subobj.data_type > 0 and subobj.bit_length > 0:
                        print("  Sub: %2.2x Datatype: %4.4x Bitlength: %4.4x Obj.access: %4.4x Name: %s"%(
                            subi, subobj.data_type, subobj.bit_length, subobj.obj_access, subobj.name))
                        if (subobj.obj_access & 0x0007) != 0:
                            print("          Value: %s"%(self.SDO2string(cnt, obj.index, subi, subobj.data_type)))

    def go_to_op_state(self):
        self.master.state_check(pysoem.SAFEOP_STATE, 50000)
        assert self.master.state == pysoem.SAFEOP_STATE

        proc_thread = threading.Thread(target=self.process_thread)
        proc_thread.start()

        self.master.state = pysoem.OP_STATE
        self.master.write_state()
        all_slaves_reached_op_state = False
        for _ in range(400):
            self.master.state_check(pysoem.OP_STATE, 50000)
            if self.master.state == pysoem.OP_STATE:
                all_slaves_reached_op_state = True
                break
        assert all_slaves_reached_op_state, 'could not reach OP state'
        self.master.in_op = True
        if not self._check_thread_handle:
            self._check_thread_handle = threading.Thread(target=self._check_thread)
            self._check_thread_handle.start()

    def process_thread(self):
        while not self._pd_thread_stop_event.is_set():
            if self._is_overlapping_enabled:
                self.master.send_overlap_processdata()
            else:
                self.master.send_processdata()
            self._actual_wkc = self.master.receive_processdata(10000)
            time.sleep(0.001)

    @staticmethod
    def _check_slave(slave, pos):
        if slave.state == (pysoem.SAFEOP_STATE + pysoem.STATE_ERROR):
            print(
                'ERROR : slave {} is in SAFE_OP + ERROR, attempting ack.'.format(pos))
            slave.state = pysoem.SAFEOP_STATE + pysoem.STATE_ACK
            slave.write_state()
        elif slave.state == pysoem.SAFEOP_STATE:
            print(
                'WARNING : slave {} is in SAFE_OP, try change to OPERATIONAL.'.format(pos))
            slave.state = pysoem.OP_STATE
            slave.write_state()
        elif slave.state > pysoem.NONE_STATE:
            if slave.reconfig():
                slave.is_lost = False
                print('MESSAGE : slave {} reconfigured'.format(pos))
        elif not slave.is_lost:
            slave.state_check(pysoem.OP_STATE)
            if slave.state == pysoem.NONE_STATE:
                slave.is_lost = True
                print('ERROR : slave {} lost'.format(pos))
        if slave.is_lost:
            if slave.state == pysoem.NONE_STATE:
                if slave.recover():
                    slave.is_lost = False
                    print(
                        'MESSAGE : slave {} recovered'.format(pos))
            else:
                slave.is_lost = False
                print('MESSAGE : slave {} found'.format(pos))

    def _check_thread(self):
        while not self._ch_thread_stop_event.is_set():
            if self.master.in_op and ((self._actual_wkc < self.master.expected_wkc) or self.master.do_check_state):
                self.master.do_check_state = False
                self.master.read_state()
                for i, slave in enumerate(self.master.slaves):
                    if slave.state != pysoem.OP_STATE:
                        self.master.do_check_state = True
                        self._check_slave(slave, i)
                if not self.master.do_check_state:
                    print('OK : all slaves resumed OPERATIONAL.')
            time.sleep(0.01)


if __name__ == "__main__":
    import sys
    available_adapters = pysoem.find_adapters()

    if len(sys.argv) < 2:
        print("Please specify an adapter! These adapters are available")
        pollAllAdapters()
        exit()
    adapter = sys.argv[1]
    if not any([adapter == aa.name for aa in available_adapters]):
        print("Requested adapter %s not found! These adapters are available"%adapter)
        pollAllAdapters()
        exit()
    print("Using adapter " + adapter)
    ec = SoemTest(adapter)

    ec.configure()
    ec.print_od()
    ec.go_to_op_state()
    print("Got to op state, starting loop")

    prevButton = None

    try:
        while True:
            for dev in ec.master.slaves:
                btn = bool(struct.unpack("<I", dev.input)[0])
                if btn != prevButton:
                    prevButton = btn
                    print("Button = " + str(btn))
                dev.output = struct.pack("<I", int(not btn))
            time.sleep(0.01)
    except KeyboardInterrupt:
        ec.__del__()
