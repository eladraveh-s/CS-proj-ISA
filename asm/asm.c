#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h> 


#define MAX_LINE_LEN 500
#define MAX_LABEL_LEN 50

char* instruction_names[22] = { "add","sub","mac","and","or","xor","sll","sra","srl","beq","bne",
                               "blt","bgt","ble","bge","jal","lw","sw","reti","in","out","halt" };

char* register_names[16] = { "$zero","$imm1","$imm2","$v0","$a0","$a1","$a2","$t0",
                           "$t1","$t2","$s0","$s1","$s2","$gp","$sp","$ra" };


struct label_node {
    char name[MAX_LABEL_LEN + 1];
    int addr;
    struct label_node* next;
};

struct label_node* label_head = NULL;

int data_memory_arr[4096] = { 0 };

int find_first_word_start(char line[]) {
    int j = 0;
    for (; j < strlen(line); j++) {
        if (!isspace((unsigned char)line[j])) {
            return j;
        }
    }
    return -1;
}

void add_label(char name[], int address) {
    struct label_node* new_label = malloc(sizeof(struct label_node));
    strcpy_s(new_label->name, MAX_LABEL_LEN + 1,name);
    new_label->addr = address;
    new_label->next = label_head;
    label_head = new_label;
}

uint64_t get_numerical_value(char str[]) {
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return (uint64_t)strtoull(str, NULL, 16);

    }
    return atoi(str);
}

void set_data_mem_arr(char address[], char data[]) {
    int numeric_address = get_numerical_value(address);
    uint64_t numeric_data = get_numerical_value(data);
    data_memory_arr[numeric_address] = numeric_data;
}

void set_data_mem_file(FILE* data_mem) {
    for (int i = 0; i < 4096; i++) {
        fprintf(data_mem, "%08X\n", data_memory_arr[i]);
    }
}

void first_pass(FILE* input_file, FILE* data_mem) {
    char line[MAX_LINE_LEN + 1];
    char first_word[MAX_LABEL_LEN + 1];
    int address = 0;

    char data_mem_address[10];
    char data_mem_data[MAX_LINE_LEN + 1];

    while (fgets(line, sizeof(line), input_file)) {
        //find fisrt word start. ignore line if the line contains only whitespaces
        int i = find_first_word_start(line);
        int j = 0;

        if (i == -1) {
            continue;
        }

        while (!isspace((unsigned char)line[i]) && line[i] != '\0') {
            first_word[j] = line[i];
            i++;
            j++;
        }
        first_word[j] = '\0';
        if (first_word[0] == '#') {
            continue;
        }

        //check if its a label, add it to the limked-list of labels
        if (first_word[j - 1] == ':') {
            first_word[j - 1] = '\0';
            add_label(first_word, address);
            continue;
        }

        //check if its a data memory set instruction
        else if (strncmp(first_word, ".word", 5) == 0) {
            while (isspace((unsigned char)line[i])) {
                i++;
            }
            j = 0;

            while (!isspace((unsigned char)line[i]) && line[i] != '\0') {
                data_mem_address[j] = line[i];
                i++;
                j++;
            }
            data_mem_address[j] = '\0';

            while (isspace((unsigned char)line[i])) {
                i++;
            }
            j = 0;

            while (!isspace((unsigned char)line[i]) && line[i] != '\0') {
                data_mem_data[j] = line[i];
                i++;
                j++;
            }
            data_mem_data[j] = '\0';

            set_data_mem_arr(data_mem_address, data_mem_data);
        }

        else {
            //we'll set the instruction memory only on the second pass 
            address++;
        }
    }

}

int get_opcode(char str[]) {
    for (int i = 0; i < 22; i++) {
        if (strcmp(instruction_names[i], str) == 0) {
            return i;
        }
    }
    return -1;
    printf("Error, couldnt find opcode"); 
}

int get_reg(char str[]) {
    for (int i = 0; i < 16; i++) {
        if (strcmp(register_names[i], str) == 0) {
            return i;
        }
    }
    return -1;
    printf("Error, couldnt find register"); 
}

int get_next_word(char line[], int i, char word[]) {
    int j = 0;

    while (isspace((unsigned char)line[i]) || line[i] == ',') {
        i++;
    }

    while (!isspace((unsigned char)line[i]) && line[i] != ',' && line[i] != '#') {
        word[j] = line[i];
        i++;
        j++;

    }
    word[j] = '\0';
    return i;
}

int get_label_address(char name[]) {
    struct label_node* node = label_head;
    while (node != NULL) {
        if (strcmp(node->name, name) == 0) {
            return node->addr;
        }
        node = node->next;
    }
}

uint64_t get_immediate(char str[]) {
    if (isalpha(str[0])) {
        return (uint64_t)get_label_address(str);
    }
    else {
        return (uint64_t)get_numerical_value(str);
    }
}

void second_pass(FILE* input_file, FILE* inst_mem) {
    char line[MAX_LINE_LEN + 1];
    char first_word[MAX_LABEL_LEN + 1];
    char word[MAX_LABEL_LEN + 1];

    uint64_t reg_mask = (uint64_t)0xF << 36;
    uint64_t immediate_mask = (uint64_t)0xFFF << 12;

    uint64_t instruction = 0;
    uint64_t opcode;
    uint64_t reg;
    uint64_t immediate;


    while (fgets(line, sizeof(line), input_file)) {
        //find fisrt word start. ignore line if the line contains only whitespaces
        int i = find_first_word_start(line);
        int j = 0;
        reg_mask = (uint64_t)0xF << 36;
        immediate_mask = 0xFFF << 12;

        if (i == -1) {
            continue;
        }

        while (!isspace((unsigned char)line[i]) && line[i] != '\0') {
            first_word[j] = line[i];
            i++;
            j++;
        }
        first_word[j] = '\0';

        if (first_word[0] == '#') {
            continue;
        }

        //check if its a label or a data memory instruction, ignore it
        if (first_word[j - 1] == ':' || strncmp(first_word, ".word", 5) == 0) {
            continue;
        }

        //else - its a SIMP instruction
        else {
            instruction = 0;
            opcode = get_opcode(first_word);
            instruction = instruction | (uint64_t)(opcode << 40);


            for (int k = 0; k < 4; k++) {
                i = get_next_word(line, i, word);
                reg = get_reg(word);
                instruction = instruction | (((reg << (36 - 4 * k)) & reg_mask));
                reg_mask = reg_mask >> 4;
            }

            for (int k = 0; k < 2; k++) {
                i = get_next_word(line, i, word);
                immediate = get_immediate(word);
                instruction = instruction | (((immediate << (12 - 12 * k)) & immediate_mask));
                immediate_mask = immediate_mask >> 12;
            }

            fprintf(inst_mem, "%012llX\n", instruction);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Invalid number of arguments");
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");
    FILE* inst_mem = fopen(argv[2], "w");
    FILE* data_mem = fopen(argv[3], "w");



    if (!input_file || !data_mem || !inst_mem) {
        perror("Error opening file(s)");
        return 1;
    }

    //in the first pass we will save label addresses as well as initiate data memory special instructions (.word instructions)
    first_pass(input_file, data_mem);
    //set up the data_mem file in the desired format using the data_mem_array we modified in first_pass()
    set_data_mem_file(data_mem);
    //rewind in order to read the file again
    rewind(input_file);

    //in the second pass we will set instruction memory for each instruction line in the assembly file, replacing labels with addresses
    second_pass(input_file, inst_mem);

    fclose(input_file);
    fclose(data_mem);
    fclose(inst_mem);

    return 0;
}