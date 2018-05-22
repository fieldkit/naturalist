#!/usr/bin/python

import subprocess
import re
import sys

class CallsDecoder:
    def read_symbols(self, elf):
        self.table = {}
        nm = subprocess.Popen(["nm", "-S", elf], stdout=subprocess.PIPE)
        process = subprocess.Popen(["c++filt"], stdin=nm.stdout, stdout=subprocess.PIPE)
        for line in iter(process.stdout.readline,''):
            fields = line.rstrip().split(" ", 3)
            if len(fields[0]) > 0:
                address = int(fields[0], 16)
                self.table[address] = fields

    def find_addresses(self, raw):
        strings = re.findall("0x([0-9a-f]+)", raw)
        return [ int(s, 16) for s in strings ]

    def reverse_addresses(self, addresses):
        for address in addresses:
            if self.table.has_key(address):
                symbol = self.table[address]
                print symbol[0], symbol[-1]

decoder = CallsDecoder()
decoder.read_symbols("build/fk-naturalist-main.elf")
for line in sys.stdin.readlines():
    addresses = decoder.find_addresses(line)
    decoder.reverse_addresses(addresses)

