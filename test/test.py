#!/usr/bin/python

import requests
import unittest
import threading
import sys

class WebServGetTest(unittest.TestCase) :
	URL = 'http://localhost:8080'
	TEST_SIZE = 100

	def setUp(self):
		if (len(sys.argv) == 2):
			self.TEST_SIZE = int(sys.argv[1])

	def get_test(self):
		res = requests.get(self.URL)
		self.assertEqual(res.status_code, 200)

	# 1회의 GET request 실행
	def test_get_basic(self):
		self.get_test()

	# test_size회의 Get request를 순서대로 수행
	def test_get_massive_sequence(self):
		for i in range(self.TEST_SIZE):
			self.get_test()

	# test_size회의 Get request를 thread를 이용해 동시에 수행
	def test_get_massive_together(self):
		for i in range(self.TEST_SIZE):
			t = threading.Thread(target=self.get_test())
			t.start()

if __name__ == '__main__':
	if len(sys.argv) <= 2:
		t = unittest.TestLoader().loadTestsFromTestCase(WebServGetTest)
		unittest.TextTestRunner(verbosity=2).run(t)
	else:
		print("Usage: ./test.py [test_size=100]")

