# webserv test
test.py를 통해 webserv 단위 테스트를 돌릴 수 있습니다.

## 준비
1. requests 모듈 설치
```python
pip install requests
```

2. webserv를 test용 conf를 이용해 실행
```shell
./webserv test.conf
```

3. test 스크립트 실행
```shell
python test.py [test_size] # ex) python test.py 100000 (test_size 기본값: 100)
```

## 테스트 종류
### GET
- 유효한 GET 1회 실행
- 유효한 GET를 연속으로 `test_size`회 실행
- 유효한 GET를 동시에(쓰레드 사용) `test_size`회 실행
- 유효하지 않은 target을 가진 GET 실행 (미구현)
### POST
미구현
### PUT
미구현
### DELETE
미구현




