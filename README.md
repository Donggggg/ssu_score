# ssu_score

## 1. 개요
 프로그래밍 실기 시험을 채점하는 작업은 사람의 손으로 직접 하면 효율성이 매우 떨어진다. 특히 코드 전체를 작성하는 문제에서 정답을 검사를 하려면 정확성도 떨어지고 시간적인 문제도 발생한다. 이를 해결하는 프로그램이 ssu_score이다. 이 프로그램은 학생들이 제출한 답안 파일을 정답 파일을 기준으로 채점을 진행한다. 채점이 지원되는 문제 종류는 빈칸 채우기 문제와 프로그램 문제이다. 빈칸 문제 채점 방식은 파스트리구조 적용해 답안과 정답을 정확히 비교한다. 프로그램 문제는 컴파일을 시켜 결과를 비교하는 방식으로 채점을 한다. 채점 기능을 제외하고 옵션으로 다양한 작업도 지원하게 된다. 이 프로그램을 개발하는 개발자는 유닉스/리눅스 컴퓨팅 환경에서의 프로그래밍 실력이 향상을 기대할 수 있다. 특히 주로 사용되는 파일 입출력, 파일속성 디렉토리에 관한 시스템 호출 함수와 라이브러리 함수를 사용하는 법에 익숙해지고 이해도가 높아질 것이다.  
 
## 2. 사용방법
```
1. git clone https://www.github.com/Donggggg/ssu_score.git
2. cd ssu_score
3. make
4. ./ssu_score <STD_DIR> <ANS_DIR> [OPTION]
```

## 3. Contribute
코드를 충분히 이해 후 적절한 옵션 기능 추가
