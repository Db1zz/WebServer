import requests
import subprocess
import socket
import filecmp
import os
from colorama import Fore, Back, Style
from time import sleep
from utils import extract_addresses_from_cfg


class Webserver:
	def __init__(self):
		self.binary_path = "./webserv"

	def _wait_server_to_start(self, args=["./Tests/global/resources/webserver_test.cfg"]):
		cfgAdresses = extract_addresses_from_cfg(args[0])
		for _ in range(50):
			with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
				result = sock.connect_ex((cfgAdresses[0][0], int(cfgAdresses[0][1])))
				if result == 0:
					sock.close()
					return
			sleep(0.1)
		return TimeoutError("Unable to start server")

	def start(self, args=["./Tests/global/resources/webserver_test.cfg"]):
		self.process = subprocess.Popen(
			self.binary_path,
			stdout=subprocess.DEVNULL,
			stderr=subprocess.DEVNULL)
		self._wait_server_to_start(args)

	def stop(self):
		self.process.terminate()

class WebserverTester:
	def __init__(self):
		self.testCases = []

	def addTestCase(self, testCase):
		self.testCases.append(testCase)

	def run(self):
		failedTestsCounter = 0
		for testCase in self.testCases:
			result, expected = testCase.test()
			if expected != result:
				failedTestsCounter += 1
				print("[" + Fore.RED + "Fail" + Style.RESET_ALL + "] " + testCase.name + ": ", \
		   			"Expected:", Fore.GREEN, expected, Style.RESET_ALL, Fore.RED, "got:", result, Style.RESET_ALL)
				continue
			print("[" + Fore.GREEN + "Pass" + Style.RESET_ALL + "] " + testCase.name, Fore.GREEN, result, Style.RESET_ALL)

class LoadExistingPage:
	name = "Load existing page"
	def test():
		serv = Webserver()
		serv.start()
		r = requests.get("http://127.0.0.1/upload.html")
		serv.stop()
		return r.status_code, 200

class LoadNonExistantPage:
	name = "Load non-existant page"
	def test():
		serv = Webserver()
		serv.start()
		r = requests.get("http://127.0.0.1/damn.html")
		serv.stop()
		return r.status_code, 404

class Load404PageWithBrokenPath:
	name = "Load 404 page with broken path"
	def test():
		serv = Webserver()
		serv.start()
		r = requests.get("http://127.0.0.1/.//damn.html")
		serv.stop()
		return r.status_code, 200

class CheckPorts:
	name = "Verify Webserver ports"
	def test():
		serv = Webserver()
		serv.start()
		cfgAddr = extract_addresses_from_cfg("./Tests/global/resources/webserver_test.cfg")
		expected = []
		result = []
		for cfgAddr in cfgAddr:
			addr = "http://" + cfgAddr[0] + ":" + cfgAddr[1]
			r = requests.get(addr)
			expected.append(cfgAddr + ("200",))
			result.append(cfgAddr + (str(r.status_code),))
		serv.stop()
		return expected, result

class CheckUpload:
	name = "Verify the uploaded file"
	def test():
		serv = Webserver()
		serv.start()
		filePath = "./Tests/global/resources/test.jpg"
		UploadedFilePath = "./Tests/global/resources/test.jpg"
		fileToUpload = open(filePath, "rb")
		files = {"file" : fileToUpload}

		r = requests.post("http://127.0.0.1/Uploads", files=files)
		expected = os.path.getsize(filePath)
		if r.ok:
			result = os.path.getsize(UploadedFilePath)
		else: 
			result = -1
		serv.stop()
		return expected, result
			


if __name__ == "__main__":

	tester = WebserverTester()

	tester.addTestCase(LoadExistingPage)
	tester.addTestCase(LoadNonExistantPage)
	tester.addTestCase(Load404PageWithBrokenPath)
	tester.addTestCase(CheckPorts)
	tester.addTestCase(CheckUpload)

	tester.run()