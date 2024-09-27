#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "document_analyzer.h"

#define INITIAL_PARAGRAPHS_CAPACITY (10)
#define INITIAL_SENTENCES_CAPACITY (10)
#define INITIAL_WORDS_CAPACITY (20)

#define BUFFER_SIZE (1024)

static document_t* s_document = NULL;

static const char*** paragraph_data = NULL;
static const char** sentence_data = NULL;

int load_document(const char* document)
{
    FILE* stream = fopen(document, "r");
    if (stream == NULL) {
        return FALSE;
    }

    dispose();

    parse_document(stream);

    fclose(stream);
    return TRUE;
}

void initialize_word(word_t* word, const char* data)
{
    word->length = strlen(data);
    word->data = (char*)malloc(word->length + 1);
    strcpy(word->data, data);
}

void add_word_to_sentence(sentence_t* sentence, const char* word_data)
{
    if (sentence->word_count == sentence->words_capacity) {
        sentence->words_capacity *= 2;
        sentence->words = (word_t**)realloc(sentence->words, sentence->words_capacity * sizeof(word_t*));
    }
    sentence->words[sentence->word_count] = (word_t*)malloc(sizeof(word_t));
    initialize_word(sentence->words[sentence->word_count], word_data);
    sentence->word_count++;
}

void add_sentence_to_paragraph(paragraph_t* paragraph, sentence_t* sentence)
{
    if (paragraph->sentence_count == paragraph->sentences_capacity) {
        paragraph->sentences_capacity *= 2;
        paragraph->sentences = (sentence_t**)realloc(paragraph->sentences, paragraph->sentences_capacity * sizeof(sentence_t*));
    }
    paragraph->sentences[paragraph->sentence_count] = sentence;
    paragraph->sentence_count++;
}

void add_paragraph_to_document(document_t* document, paragraph_t* paragraph)
{
    if (document->paragraph_count == document->paragraphs_capacity) {
        document->paragraphs_capacity *= 2;
        document->paragraphs = (paragraph_t**)realloc(document->paragraphs, document->paragraphs_capacity * sizeof(paragraph_t*));
    }
    document->paragraphs[document->paragraph_count] = paragraph;
    document->paragraph_count++;
}

void parse_document(FILE* stream)
{
    char line[BUFFER_SIZE];

    s_document = (document_t*)malloc(sizeof(document_t));
    s_document->paragraphs_capacity = INITIAL_PARAGRAPHS_CAPACITY;
    s_document->paragraph_count = 0;
    s_document->paragraphs = (paragraph_t**)malloc(s_document->paragraphs_capacity * sizeof(paragraph_t*));

    while (fgets(line, sizeof(line), stream) != NULL) {
        paragraph_t* paragraph;
        char copied_line[BUFFER_SIZE];
        char* tokenized_sentence;
        char* ptr_copied_line;

        if (strcmp(line, "\n") == 0) {
            continue;
        }

        paragraph = (paragraph_t*)malloc(sizeof(paragraph_t));
        paragraph->sentences_capacity = INITIAL_SENTENCES_CAPACITY;
        paragraph->sentence_count = 0;
        paragraph->sentences = (sentence_t**)malloc(paragraph->sentences_capacity * sizeof(sentence_t*));

        strcpy(copied_line, line);
        ptr_copied_line = copied_line;
        tokenized_sentence = strtok(copied_line, ".!?");

        while (tokenized_sentence != NULL) {
            sentence_t* sentence;
            char copied_sentence[BUFFER_SIZE];
            char* tokenized_word;

            sentence = (sentence_t*)malloc(sizeof(sentence_t));
            sentence->words_capacity = INITIAL_WORDS_CAPACITY;
            sentence->word_count = 0;
            sentence->words = (word_t**)malloc(sentence->words_capacity * sizeof(word_t*));

            strcpy(copied_sentence, tokenized_sentence);
            tokenized_word = strtok(copied_sentence, " ,");
            while (tokenized_word != NULL) {
                add_word_to_sentence(sentence, tokenized_word);
                tokenized_word = strtok(NULL, " ,");
            }

            add_sentence_to_paragraph(paragraph, sentence);

            ptr_copied_line += strlen(tokenized_sentence) + 1;
            if (*(ptr_copied_line) == '\n') {
                break;
            }
            tokenized_sentence = strtok(ptr_copied_line, ".!?");
        }

        add_paragraph_to_document(s_document, paragraph);
    }
}

void free_word(word_t* word)
{
    free(word->data);
}

void free_sentence(sentence_t* sentence)
{
    size_t i;
    for (i = 0; i < sentence->word_count; ++i) {
        free_word(sentence->words[i]);
        free(sentence->words[i]);
    }
    free(sentence->words);
}

void free_paragraph(paragraph_t* paragraph)
{
    size_t i;
    for (i = 0; i < paragraph->sentence_count; ++i) {
        free_sentence(paragraph->sentences[i]);
        free(paragraph->sentences[i]);
    }
    free(paragraph->sentences);
}

void free_document(document_t* document)
{
    size_t i;
    for (i = 0; i < document->paragraph_count; ++i) {
        free_paragraph(document->paragraphs[i]);
        free(document->paragraphs[i]);
    }
    free(document->paragraphs);

    free(document);
}

void dispose(void)
{
    {
        if (s_document != NULL) {
            free_document(s_document);
            s_document = NULL;
        }
    }

    {
        if (paragraph_data != NULL) {
            size_t i;
            for (i = 0; paragraph_data[i] != NULL; ++i) {
                free(paragraph_data[i]);
            }
            free(paragraph_data);
            paragraph_data = NULL;
        }
    }

    {
        if (sentence_data != NULL) {
            free(sentence_data);
            sentence_data = NULL;
        }
    }
}

unsigned int get_total_word_count(void)
{
    if (s_document == NULL) {
        return 0;
    }

    {
        size_t i;
        size_t j;
        unsigned int total_word_count = 0;
        size_t paragraph_count = s_document->paragraph_count;
        for (i = 0; i < paragraph_count; ++i) {
            paragraph_t* paragraph = s_document->paragraphs[i];
            size_t sentence_count = paragraph->sentence_count;
            for (j = 0; j < sentence_count; ++j) {
                total_word_count += paragraph->sentences[j]->word_count;
            }
        }

        return total_word_count;
    }
}

unsigned int get_total_sentence_count(void)
{
    if (s_document == NULL) {
        return 0;
    }

    {
        int i;
        unsigned int total_sentence_count = 0;
        size_t paragraph_count = s_document->paragraph_count;
        for (i = 0; i < paragraph_count; ++i) {
            total_sentence_count += s_document->paragraphs[i]->sentence_count;
        }
        return total_sentence_count;
    }
}

unsigned int get_total_paragraph_count(void)
{
    if (s_document == NULL) {
        return 0;
    }

    return s_document->paragraph_count;
}

const char*** get_paragraph_or_null(const unsigned int paragraph_index)
{
    if (s_document == NULL || s_document->paragraph_count == 0) {
        return NULL;
    }

    if (paragraph_index >= s_document->paragraph_count) {
        return NULL;
    }

    {
        size_t i;
        size_t j;
        if (paragraph_data != NULL) {
            for (i = 0; paragraph_data[i] != NULL; ++i) {
                free(paragraph_data[i]);
            }
            free(paragraph_data);
            paragraph_data = NULL;
        }

        {
            paragraph_t* paragraph = s_document->paragraphs[paragraph_index];
            size_t sentence_count = paragraph->sentence_count;
            paragraph_data = (const char***)malloc((sentence_count + 1) * sizeof(const char**));

            for (i = 0; i < paragraph->sentence_count; ++i) {
                sentence_t* sentence = paragraph->sentences[i];
                size_t word_count = sentence->word_count;
                paragraph_data[i] = (const char**)malloc((word_count + 1) * sizeof(const char*));

                for (j = 0; j < sentence->word_count; ++j) {
                    paragraph_data[i][j] = sentence->words[j]->data;
                }
                paragraph_data[i][word_count] = NULL;
            }
            paragraph_data[sentence_count] = NULL;

            return paragraph_data;
        }
    }
}

unsigned int get_paragraph_word_count(const char*** paragraph)
{
    size_t i;
    size_t j;
    unsigned int word_count = 0;
    for (i = 0; paragraph[i] != NULL; ++i) {
        const char** sentence = paragraph[i];
        for (j = 0; sentence[j] != NULL; ++j) {
            ++word_count;
        }
    }
    return word_count;
}

unsigned int get_paragraph_sentence_count(const char*** paragraph)
{
    unsigned int sentence_count = 0;

    if (s_document == NULL) {
        return sentence_count;
    }

    {
        sentence_t** sentences = (sentence_t**)paragraph;
        while (sentences[++sentence_count] != NULL) {
        }
    }
    return sentence_count;
}

const char** get_sentence_or_null(const unsigned int paragraph_index, const unsigned int sentence_index)
{
    if (s_document == NULL || s_document->paragraph_count == 0) {
        return NULL;
    }

    if (paragraph_index >= s_document->paragraph_count
        || sentence_index >= s_document->paragraphs[paragraph_index]->sentence_count) {
        return NULL;
    }

    if (sentence_data != NULL) {
        free(sentence_data);
        sentence_data = NULL;
    }

    {
        paragraph_t* paragraph = s_document->paragraphs[paragraph_index];
        if (sentence_index >= paragraph->sentence_count) {
            return NULL;
        }

        {
            sentence_t* sentence = paragraph->sentences[sentence_index];
            sentence_data = (const char**)malloc((sentence->word_count + 1) * sizeof(const char*));
            {
                size_t i;
                for (i = 0; i < sentence->word_count; ++i) {
                    word_t* word = sentence->words[i];
                    sentence_data[i] = word->data;
                }
                sentence_data[sentence->word_count] = NULL;

                return sentence_data;
            }
        }
    }
}

unsigned int get_sentence_word_count(const char** sentence)
{
    unsigned int word_count = 0;

    if (s_document == NULL) {
        return word_count;
    }

    while (sentence[++word_count] != NULL) {
    }
    return word_count;
}

int print_as_tree(const char* filename)
{
    if (s_document == NULL || s_document->paragraph_count == 0) {
        return FALSE;
    }

    {
        FILE* stream;
        stream = fopen(filename, "w");

        {
            size_t i;
            size_t j;
            size_t k;

            for (i = 0; i < s_document->paragraph_count; ++i) {
                paragraph_t* paragraph = s_document->paragraphs[i];

                if (i != 0) {
                    fprintf(stream, "\n");
                    fprintf(stream, "\n");
                }
                fprintf(stream, "Paragraph %zu:", i);

                for (j = 0; j < paragraph->sentence_count; ++j) {
                    sentence_t* sentence = paragraph->sentences[j];

                    fprintf(stream, "\n");
                    fprintf(stream, "    Sentence %zu:", j);

                    for (k = 0; k < sentence->word_count; ++k) {
                        fprintf(stream, "\n");
                        fprintf(stream, "        %s", sentence->words[k]->data);
                    }
                }
            }
        }

        fclose(stream);
        return TRUE;
    }
}
