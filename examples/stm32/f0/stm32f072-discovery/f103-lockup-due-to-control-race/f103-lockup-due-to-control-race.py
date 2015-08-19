"""
Copyright (C) 2015 Kuldeep Singh Dhaka <kuldeepdhaka9@gmail.com>
Licence: GPLv3 or later
"""

import usb.core
import sys
import threading

TRIAL = 5000
TIMEOUT = 1000 #milliseconds

class UsbThread(threading.Thread):
	def __init__(self, dev):
		self.dev = dev
		self.success = 0
		self.trial = 0
		self.stop = False
		threading.Thread.__init__(self)

	def request_stop(self):
		self.stop = True

class ControlThread(UsbThread):
	def run(self):
		success = 0
		i = 0
		while i < TRIAL and not self.stop:
			try:
				dev.ctrl_transfer(0xC0, 0, timeout=TIMEOUT, data_or_wLength=100)
				res = "SUCCESS"
				success += 1
			except:
				res = "FAILED"
			finally:
				print("control: trial %i: %s" % (i, res))
			i = i + 1
		self.trial = i
		self.success = success

class BulkThread(UsbThread):
	def run(self):
		success = 0
		i = 0
		arr = bytearray(64)
		while i < TRIAL and not self.stop:
			try:
				dev.write(0x01, arr, timeout=TIMEOUT)
				res = "SUCCESS"
				success += 1
			except:
				res = "FAILED"
			finally:
				print("bulk: trial %i: %s" % (i, res))
			i = i + 1
		self.trial = i
		self.success = success

def find_percent(val, total):
	return (val * 100.0) / total

def print_stat(name, obj):
	perc = find_percent(obj.success, obj.trial)
	print("%s: success %i out of %i (%.2f%%)" % (name, obj.success, obj.trial, perc))

if __name__ == "__main__":
	dev = usb.core.find(idVendor=0xcafe, idProduct=0xcafe)
	if dev is None:
		raise ValueError('Device not found')

	dev.set_configuration()
	dev.set_interface_altsetting()

	control = ControlThread(dev)
	bulk = BulkThread(dev)

	control.start()
	bulk.start()

	try:
		bulk.join()
		control.request_stop()
		control.join()
	except KeyboardInterrupt:
		control.request_stop()
		bulk.request_stop()
		bulk.join()
		control.join()

	print_stat("control", control)
	print_stat("bulk", bulk)
