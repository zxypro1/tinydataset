#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute) // 取struct中的一个属性
#define TABLE_MAX_PAGES 100

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row; // 行信息 这里直接hard-code了

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
} Table;

void serialize_row(Row* source, void* destination) { // 序列化，将source的数据存入destination
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) { // 反序列化
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer; // 创建输入buffer数据结构

InputBuffer* new_input_buffer() { // 新建输入数据
    InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->input_length = 0;
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;

    return input_buffer;
}

void print_prompt() { printf("db > "); } // 命令行显示

void read_input(InputBuffer* input_buffer) { // 读取输入
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0) { // 读取出问题
        printf("Error reading input");
        exit(EXIT_FAILURE);
    }

    // 去掉输入最后一行的换行符
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) { // 释放内存
    free(input_buffer->buffer);
    free(input_buffer);
}

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult; // 元命令读取结果

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult; // 元命令准备结果

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType; // 两种指令

typedef struct {
    StatementType type;
    Row row_to_insert; //只用于insert操作
} Statement;

MetaCommandResult  do_meta_command(InputBuffer* input_buffer) { // 执行元命令
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS); // TODO: 运行内容
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) { // 理解语句。目前只有select和insert
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) { // 这里使用strncmp是因为insert语句后面会接数据（Num）
        statement->type = STATEMENT_INSERT;
        int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
                                   statement->row_to_insert.username, statement->row_to_insert.email);
        if (args_assigned < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT; // 读取失败
}

void execute_statement(Statement* statement) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            printf("Do insert"); // TODO：这里插入"insert"的操作
            break;
        case (STATEMENT_SELECT):
            printf("Do select"); // TODO：这里插入"select"的操作
            break;
    }
}

int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();
    while (true){
        print_prompt();
        read_input(input_buffer);

        
        
        if (strcmp(input_buffer->buffer, ".exit") == 0) { // 不是退出，运行指令
            if (input_buffer->buffer[0] == '.') {
                switch (do_meta_command(input_buffer)) { // 前面带.的叫元命令
                    case (META_COMMAND_SUCCESS):
                        continue;
                    case (META_COMMAND_UNRECOGNIZED_COMMAND):
                        printf("Unrecognized command '%s'\n", input_buffer->buffer);
                        continue;
                }
            }

            Statement statement;
            switch (prepare_statement(input_buffer, &statement)) {
                case (PREPARE_SUCCESS):
                    break;
                case (PREPARE_UNRECOGNIZED_STATEMENT):
                    printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                    continue;
            }

            execute_statement(&statement);
            printf("Executed.\n");
        }
    }
}
