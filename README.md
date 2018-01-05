Simple Open Source EtherCAT Slave
====
[![Build Status](https://travis-ci.org/OpenEtherCATsociety/SOES.svg?branch=master)](https://travis-ci.org/OpenEtherCATsociety/SOES)

SOES (Simple OpenSource EtherCAT Slave Stack) is an opensource slave
stack that is very easy to use and provides a small footprint. It is a
good alternative to more complex stacks on the market.

Overview
----
SOES is an EtherCAT slave stack written in c. Its purpose is to learn and
to use. All users are invited to study the source to get an understanding
how an EtherCAT slave functions.

Features as of 2.1.3:
 - Address offset based HAL for easy ESC read/write access via any
   interface
 - Mailbox with data link layer
 - CoE
 - Object dictionary
 - SDO read and write for all sizes including segmented transfers
 - Easy portable C-code suited for embedded applications
 - Fixed PDO mapping
 - FoE with bootstrap template
 - Support for Little and Big endian targets
 - Run polling, mixed polling/interrupt or interrupt
 - Support for SM Synchronization 
 - Support DC sync0 and DC Synchronization
 - Add stack configuration via new configuration paramater to/or from 
   "stack"_init

TODO
 - Update documentation
