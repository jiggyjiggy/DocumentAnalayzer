# DocumentAnalayzer

간단히 텍스트를 읽거나 검색할 때는 하나의 긴 문자 배열로 메모리에 저장하는 정도면 충분하다. <br>
하지만 파일 속에 있는 텍스트를 좀 더 면밀하게 분석하고 싶다면 문자 배열보다는 복잡한 구조가 필요하다. 보고서 같이 여러 문단으로 구성된 문서를 분석한다고 생각해보자. 그 문서 안에 들어간 단어(word) 수, 문장(sentence) 수, 문단(paragraph) 수와 같은 정보가 필요하다. <br>
이런 일을 위해 각 단어를 토큰으로 만든 뒤, 다음과 같은 중첩(nested) 구조에 저장하자
```
                                  document  
                             /        |       \  
                    paragraph1    paragraph2   paragraph3  
                    /       |         |           |       \  
             sentence1   sentence2  sentence1   sentence1  sentence2  
          /    |     \      |    \         \          |         \  
      word1  word2  word3  word1 word2    word1     word1       word1  
```

이 구조는 문장은 여러 개의 단어(그리고 구두점)들을 합쳐서 형성되며, 문단은 여러 개의 문장들을, 문서는 여러 개의 문단들을 합쳐서 형성됨을 보여준다. 이것을 C 포인터를 사용해 표현하면 다음과 같다.
- 단어 1개는 `char*`
- 단어 목록, 즉 문장 하나는 `char**`
- 문장 목록, 즉 문단 하나는 `char***`
- 문단 목록, 즉 문서 하나는 `char****`

다중 포인터와 동적 할당을 인지하면서, 작업에 임하자

## 가정한 것들
- 문서는 올바른 문법과 구두점을 사용한다
- 문단들은 최소한 하나의 새 줄 문자(`\n`)로 구분되어 있다
- 문장은 `.`, `!`, `?` 중의 한 문자로 구분되어 있다
- 단어들은 빈칸(스페이스)이나 `,` 문자로 구분되어 있다

## 핵심 설계 사항
### 힙 메모리 할당 시점 최소화
힙 메모리 할당은 매우 느린 작업에 속한다. 따라서, 데이터의 변경마다 매번 하는 것은 매우 치명적이라고 생각한다. <br>
따라서, capacity, size 전략으로 메모리 할당 횟수 최소화했다. <br>
다만, 2배씩 커지는 정책으로 임의로 했다. 성능 측정을 통해서 최적의 정책을 찾아야 할 것이다.

## 지원 함수 및 설명

### `int load_document(const char* document);`
- `load_document()` 함수는 다음의 인자를 받는다.
    - 문서 파일의 이름: `const char* document`
- 주어진 문서의 내용을 읽어 메모리에 저장한다.
- 문서를 성공적으로 메모리에 읽어왔다면 `TRUE`를, 아니라면 `FALSE`를 반환한다.

### `void dispose(void);`
- `dispose()` 함수는 아무 인자도 받지 않는다.
- 동적으로 할당된 메모리를 모두 해제한다.

### `unsigned int get_total_word_count(void);`
- `get_total_word_count()` 함수는 아무 인자도 받지 않는다.
- 로딩된 문서 안에 있는 총 단어 수를 반환한다.
- 로딩된 문서가 없다면 0을 반환한다.

### `unsigned int get_total_sentence_count(void);`
- `get_total_sentence_count()` 함수는 아무 인자도 받지 않는다.
- 로딩된 문서 안에 있는 총 문장 수를 반환한다.
- 로딩된 문서가 없다면 0을 반환한다.

### `unsigned int get_total_paragraph_count(void);`
- `get_total_paragraph_count()` 함수는 아무 인자도 받지 않는다.
- 로딩된 문서 안에 있는 총 문단 수를 반환한다.
- 로딩된 문서가 없다면 0을 반환한다.

### `const char*** get_paragraph_or_null(const unsigned int paragraph_index);`
- `get_paragraph_or_null()` 함수는 다음의 인자를 받는다.
    - 문단 목록에서 찾을 문단의 색인: `const unsigned int paragraph_index`
- 지정된 색인에 있는 문단을 반환한다.
- 범위 밖의 색인이 들어오거나 로딩된 문서가 없다면 `NULL`을 반환한다.

### `unsigned int get_paragraph_word_count(const char*** paragraph);`
- `get_paragraph_word_count()` 함수는 다음의 인자를 받는다.
    - 문단: `const char*** paragraph`
- 주어진 문단 속에 있는 단어의 개수를 반환한다.

### `unsigned int get_paragraph_sentence_count(const char*** paragraph);`
- `get_paragraph_sentence_count()` 함수는 다음의 인자를 받는다.
- 문단: `const char*** paragraph`
- 주어진 문단 속에 있는 문장의 개수를 반환한다.

### `const char** get_sentence_or_null(const unsigned int paragraph_index, const unsigned int sentence_index);`
- `get_sentence_or_null()` 함수는 다음의 인자를 받는다.
    - 문단 목록에서 찾을 문단의 색인: `const unsigned int paragraph_index`
    - 해당 문단속에서 찾을 문장의 색인: `const unsigned int sentence_index`
- 지정된 문단 색인과 문장 색인에 있는 문장을 반환한다.
- 범위 밖의 색인들이 들어오거나 로딩된 문서가 없다면 `NULL`을 반환한다.

### `unsigned int get_sentence_word_count(const char** sentence);`
- `get_sentence_word_count()` 함수는 다음의 인자를 받는다.
    - 문장: `const char** sentence`
- 주어진 문장 속에 있는 단어의 개수를 반환한다.

### `int print_as_tree(const char* filename);`
- `print_as_tree()` 함수는 다음의 인자를 받는다.
- 트리를 출력할 파일명: `const char* filename`
- 문서의 트리 형태를 `filename` 파일에 저장합니다. 트리 형태는 다음과 같다.
    - 각 문단마다 "Paragraph < 문단 색인 >:"을 출력합니다. 각 문단은 빈 줄 하나를 넣어 구분한다.
    - 한 문단에 속한 각 문장은 스페이스 4개만큼 들여 쓴 뒤, "Sentence < 문장 색인 >:"을 출력한다.
    - 한 문장에 속한 각 단어는 추가로 스페이스 4개만큼 들여 쓴 뒤, 단어를 출력합니다. 이때 한 줄에 한 단어만을 출력한다.
- 파일에 성공적으로 출력했다면 `TRUE`를, 아니라면 `FALSE`를 반환한다.
