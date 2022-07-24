#!/usr/bin/python

import requests
import unittest
import threading
import sys

class WebServGetTest(unittest.TestCase) :
	URL = 'http://localhost:8080'
	SUB_URL = 'http://localhost:8081'
	TEST_SIZE = 100

	def setUp(self):
		if (len(sys.argv) == 2):
			self.TEST_SIZE = int(sys.argv[1])

	def test_200_ok_get(self):
		res = requests.get(self.URL)
		self.assertEqual(res.status_code, 200)

	def test_200_ok_head(self):
		res = requests.head(self.URL + "/put_test")
		self.assertEqual(res.status_code, 200)

	def test_201_created_put(self):
		res = requests.put(self.URL + "/put_test/test_temp_output", data="201_created_put")
		self.assertEqual(res.status_code, 201)

	def test_204_post_exist(self):
		res = requests.post(self.URL + "/post_body/test_temp_output", data="204_post_exist")
		self.assertEqual(res.status_code, 204)

	def test_204_post_new(self):
		res = requests.post(self.URL + "/post_body/not_exist", data="204_post_new")
		self.assertEqual(res.status_code, 204)
	
	def test_204_put_exist(self):
		res = requests.put(self.URL + "/put_test/test_temp_output", data="204_put_exist")
		self.assertEqual(res.status_code, 204)

	def test_204_delete(self):
		res = requests.delete(self.URL + "/directory/test_temp_output")
		self.assertEqual(res.status_code, 204)

	def test_400_no_method_exist(self):
		res = requests.request("NO_METHOD", self.URL + "/put_test/test_temp_output")
		self.assertEqual(res.status_code, 400)

	def test_400_no_method_new(self):
		res = requests.request("NO_METHOD", self.URL + "/put_test/not_exist")
		self.assertEqual(res.status_code, 400)

	def test_404_not_found_get(self):
		res = requests.get(self.URL + "/i_dont_have_this_page")
		self.assertEqual(res.status_code, 404)

	def test_404_not_found_head(self):
		res = requests.head(self.URL + "/put_test/i_dont_have_this_page")
		self.assertEqual(res.status_code, 404)

	def test_405_method_not_allowed_get(self):
		res = requests.get(self.URL + "/post_body/index.html")
		self.assertEqual(res.status_code, 405)
		
	def test_405_method_not_allowed_post(self):
		res = requests.post(self.URL)
		self.assertEqual(res.status_code, 405)

	def test_411_length_required_post(self):
		res = requests.request("POST", self.URL + "/post_body/index.html", headers={"Content-Length":""})
		self.assertEqual(res.status_code, 411)

	def test_411_length_required_put(self):
		res = requests.request("PUT", self.URL + "/put_test/new.html", headers={"Content-Length":""})
		self.assertEqual(res.status_code, 411)

	def test_413_payload_too_large_post(self):
		res = requests.post(self.URL + "/post_body/index.html", data=("a"*10000))
		self.assertEqual(res.status_code, 413)

	def test_virtual_hosting_1(self):
		res = requests.get(self.URL, headers={"Host":"webserv"})
		self.assertEqual(res.status_code, 200)

	def test_virtual_hosting_2(self):
		res = requests.get(self.URL, headers={"Host":"webserv1"})
		self.assertEqual(res.status_code, 200)

	def get_test(self):
		res = requests.get(self.URL)
		self.assertEqual(res.status_code, 200)

	def test_get_basic(self):
		self.get_test()

	# def test_get_massive_sequence(self):
	# 	for i in range(self.TEST_SIZE):
	# 		self.get_test()

	# def test_get_massive_together(self):
	# 	for i in range(self.TEST_SIZE):
	# 		t = threading.Thread(target=self.get_test())
	# 		t.start()
	
if __name__ == '__main__':
	if len(sys.argv) <= 2:
		t = unittest.TestLoader().loadTestsFromTestCase(WebServGetTest)
		unittest.TextTestRunner(verbosity=2).run(t)
	else:
		print("Usage: ./test.py [test_size=100]")

