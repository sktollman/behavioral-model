#!/usr/bin/env python

import argparse
import sys
import socket
import random
import struct
import re

from scapy.all import *
import readline

CALC_TYPE = 0x1212

ADD_OP     =  0
SUB_OP     =  1
LOOKUP_OP  =  2
ADD_REG_OP =  3
SET_REG_OP =  4

class Calc(Packet):
    name = "Calc"
    fields_desc = [
        IntField("op1", 0),
        ByteEnumField("opCode", 0, {ADD_OP:"ADD", SUB_OP:"SUB", LOOKUP_OP:"LOOKUP", ADD_REG_OP:"ADD_REG", SET_REG_OP:"SET_REG"}),
        IntField("op2", 0),
        IntField("result", 0)
    ]
    def mysummary(self):
        return self.sprintf("op1=%op1% %opCode% op2=%op2% result=%result%")


bind_layers(Ether, Calc, type=CALC_TYPE)
bind_layers(Calc, Raw)

class NumParseError(Exception):
    pass

class OpParseError(Exception):
    pass

class Token:
    def __init__(self,type,value = None):
        self.type = type
        self.value = value

def num_parser(s, i, ts):
    pattern = "^\s*([0-9]+)\s*"
    match = re.match(pattern,s[i:])
    if match:
        ts.append(Token('num', match.group(1)))
        return i + match.end(), ts
    raise NumParseError('Expected number literal.')


def op_parser(s, i, ts):
    pattern = "^\s*([-+&|^])\s*"
    match = re.match(pattern,s[i:])
    if match:
        ts.append(Token('num', match.group(1)))
        return i + match.end(), ts
    raise NumParseError("Expected binary operator '-', '+', '&', '|', or '^'.")


def make_seq(p1, p2):
    def parse(s, i, ts):
        i,ts2 = p1(s,i,ts)
        return p2(s,i,ts2)
    return parse

from threading import Thread
def print_all_traffic():
    sniff(prn=lambda p: p.show())


def main():

    # print_thread = Thread(target=print_all_traffic)
    # print_thread.setDaemon(True)
    # print_thread.start()

    p = make_seq(num_parser, make_seq(op_parser,num_parser))
    s = ''
    iface = 'h1-eth0'

    # sendp(Ether(dst='00:04:00:00:00:01', src='00:04:00:00:00:00'))

    # return

    while True:
        s = str(raw_input('> '))
        if s == "quit":
            break
        print s
        try:
            i,ts = p(s,0,[])
            op = 0 if ts[1].value == '+' else 2
            pkt = Ether(dst='00:04:00:00:00:01', src='00:04:00:00:00:00') / Calc(
                                              op1=int(ts[0].value),
                                              opCode=op,
                                              op2=int(ts[2].value))
            pkt = pkt/' '

            pkt.show()
            resp = srp1(pkt, iface='eth0', timeout=1, verbose=False)
            if resp:
                p4calc=resp[Calc]
                if p4calc:
                    print p4calc.result
                else:
                    print "cannot find P4calc header in the packet"
            else:
                print "Didn't receive response"
        except Exception as error:
            raise(error)
            print error


if __name__ == '__main__':
    main()
