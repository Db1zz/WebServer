import requests
import re
import subprocess
from colorama import Fore, Back, Style
from time import sleep

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
		r = requests.get("http://127.0.0.1/upload.html")
		return r.status_code, 200

class LoadNonExistantPage:
	name = "Load non-existant page"
	def test():
		r = requests.get("http://127.0.0.1/damn.html")
		return r.status_code, 404

class Load404PageWithBrokenPath:
	name = "Load 404 page with broken path"
	def test():
		r = requests.get("http://127.0.0.1/.//damn.html")
		return r.status_code, 200

class CheckPorts:
	name = "Check Webserver ports"
	def test():
		cfgFile = open("./Tests/webserver_test.cfg")
		cfgContent = cfgFile.read()
		cfgPorts = re.findall(r'listen\s+([\d\.]+):(\d+);', cfgContent)
		expected = []
		result = []
		for cfgPort in cfgPorts:
			addr = "http://" + cfgPort[0] + ":" + cfgPort[1]
			r = requests.get(addr)
			expected.append(cfgPort + ("200",))
			result.append(cfgPort + (str(r.status_code),))

		cfgFile.close()
		return expected, result

if __name__ == "__main__":
	binary_path = "./webserv"
	args = [""]
	process = subprocess.Popen(
		binary_path,
		stdout=subprocess.DEVNULL,
		stderr=subprocess.DEVNULL
	)

	sleep(2)

	tester = WebserverTester()

	tester.addTestCase(LoadExistingPage)
	tester.addTestCase(LoadNonExistantPage)
	tester.addTestCase(Load404PageWithBrokenPath)
	tester.addTestCase(CheckPorts)

	tester.run()

	process.terminate()