#!/usr/bin/python

# 
#
#
#


def main():

	subprocess.Popen('/home/meru/bmistree/new-craq-dist/craq-router-32 -d meru -p 10499 -z 192.168.1.30:9888', shell=True)
	subprocess.Popen('/home/meru/bmistree/new-craq-dist/craq-router-32 -d meru -p 10498 -z 192.168.1.30:9888', shell=True)


        return 0



if __name__ == "__main__":

        import sys
        import subprocess
	import time
        sys.exit(main())
