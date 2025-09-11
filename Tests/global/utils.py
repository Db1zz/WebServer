import re

def extract_addresses_from_cfg(cfgPath):
	cfgFile = open(cfgPath)
	cfgContent = cfgFile.read()
	cfgAdresses = re.findall(r'listen\s+([\d\.]+):(\d+);', cfgContent)
	cfgFile.close()
	return cfgAdresses