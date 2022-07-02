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

	def test_get_basic(self):
		self.get_test()

	def test_get_massive_sequence(self):
		for i in range(self.TEST_SIZE):
			self.get_test()

	def test_get_massive_together(self):
		for i in range(self.TEST_SIZE):
			t = threading.Thread(target=self.get_test())
			t.start()

	def test_204_no_content(self):
		res = requests.get(self.URL)
		if res.content == "":
			self.assertEqual(res.status_code, 204)

	# 400 Bad Request: 이 응답은 잘못된 문법으로 인하여 서버가 요청하여 이해할 수 없음을 의미합니다.
	def test_400_bad_request(self):
		headers={'Connection':''}
		res = requests.get(self.URL, headers=headers)
		print(res.request.headers)
		self.assertEqual(res.status_code, 400)

	# def test_403_forbidden(self):
	# 	res = requests.get(self.URL)
	#	self.assertEqual(res.status_code, 403)

	def test_404_not_found(self):
		HTML_URL = f'{self.URL}/kk'
		res = requests.get(HTML_URL)
		self.assertEqual(res.status_code, 404)

	# localhost:8080/ 은 GET만 가능
	def test_405_method_not_allowed(self):
		res = requests.put(self.URL)
		self.assertEqual(res.status_code, 405)

	def test_408_request_timeout(self):
		res = requests.get(self.URL, timeout=100)
		self.assertEqual(res.status_code, 408)

	# def test_411_length_required(self):
	# 	res = requests.get(self.URL)
	# 	self.assertEqual(res.status_code, 411)

	# def test_429_too_many_requests(self):
	# 	res = requests.get(self.URL)
	# 	self.assertEqual(res.status_code, 429)

	# def test_500_internal_server_error(self):
	# 	res = requests.get(self.URL)
	# 	self.assertEqual(res.status_code, 500)

	# def test_501_not_implemented(self):
	# 	res = requests.get(self.URL)
	# 	self.assertEqual(res.status_code, 501)

	# def test_test(self):
	# 	headers = {'Connection':'test'}
	# 	res = requests.request('GET', url=self.URL, headers=headers)
	# 	print(res.request)
	
if __name__ == '__main__':
	if len(sys.argv) <= 2:
		t = unittest.TestLoader().loadTestsFromTestCase(WebServGetTest)
		unittest.TextTestRunner(verbosity=2).run(t)
	else:
		print("Usage: ./test.py [test_size=100]")

