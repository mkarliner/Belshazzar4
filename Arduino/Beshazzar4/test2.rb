#!/usr/bin/ruby


require 'rubygems'
require 'serialport'


sp = SerialPort.new("/dev/tty.usbserial-A600aulh", 9600, 8, 1, SerialPort::NONE)
sleep(10)

  puts sp.puts("F\n")
  sleep(10)
  puts sp.gets
  



