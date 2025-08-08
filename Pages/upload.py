import os
filenames = os.listdir('./Uploads')
logFile = open('filenames.txt')
for name in filenames:
    logFile.write(filename)
logFile.close()