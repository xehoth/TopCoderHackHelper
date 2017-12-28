/*******************************************************************************
 * Copyright (c) 2017, xehoth
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
#include "EthernetHeader.h"

// 题目名前 1 个 '\0' 和 8 个 0xff，10 个不可见字符，最后一个 int 题目名称长度
const u_char MATCH_KEYWORD[] = {0x00, 0xff, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0xff, 0x00};
const int MATCH_KEYWORD_LENGTH = 9;
const int OFFSET_BEFORE_PROBLEM_NAME = 10 + MATCH_KEYWORD_LENGTH;

// 代码
char *code = nullptr;
int codeIndex = 0;

// 题目名称后有 40 个不可见字符，最后一个 int 用户名称长度
const int OFFSET_BEFORE_USER_NAME = 40;
// 接下来是用户名，接下来有 9 个不可见字符
// (字符串结尾 '\0' 和 2 个 int, 最后一个 int 是代码长度)
const int OFFSET_BEFORE_CODE = 1 + 2 * 4;

char userName[1024];
char problemName[1024];

int callBack(const u_char *packet, int size, int lastlen) {
    if (lastlen >= 0) {
        int i = 0;
        while (lastlen >= 0 && i < size) {
            code[codeIndex++] = packet[i];
            i++;
            lastlen--;
        }

        if (lastlen <= 0) {
            code[codeIndex] = 0;
            printf("\n------------------------end------------------------\n");
            static int file_id = 0;
            char filename[100];
            problemName[strlen(problemName)] = '_';
            strcpy(filename, strcat(problemName, userName));
            strcat(filename, ".txt");
            FILE *fp = fopen(filename, "w");
            for (int o = 0; o < codeIndex; o++) fprintf(fp, "%c", code[o]);
            long curTime = time(0);
            fprintf(fp, "\n//%s\n", ctime(&curTime));
            fclose(fp);
            chmod(filename, 0777);
            printf("code saved to file %s\n", filename);
            delete[] code;
            return -1;
        }
        return lastlen;
    }
    int flag = -1;
    for (int i = 0; i < size - MATCH_KEYWORD_LENGTH; ++i) {
        int j = 0;
        int curI = i;
        for (; j < MATCH_KEYWORD_LENGTH; j++, curI++) {
            if (packet[curI] != MATCH_KEYWORD[j]) {
                if (j > MATCH_KEYWORD_LENGTH - 1) {
                    std::cerr << "MATCH SUCCEED\n";
                }
                break;
            }
        }
        if (j >= MATCH_KEYWORD_LENGTH) {
            flag = i;
            break;
        }
    }
    if (flag != -1) {
        int i = flag;
        i += OFFSET_BEFORE_PROBLEM_NAME;
        printf("\n----------%s --- by %s---------", packet + i,
               packet + i + (*((u_char *)(packet + i - 1))) +
                   OFFSET_BEFORE_USER_NAME);
        memset(problemName, 0, sizeof(problemName));
        strcpy(problemName, (char *)packet + i);
        memset(userName, 0, sizeof(userName));
        strcpy(userName, (char *)packet + i + (*((u_char *)(packet + i - 1))) +
                             OFFSET_BEFORE_USER_NAME);
        i += (*((u_char *)(packet + i - 1))) + OFFSET_BEFORE_USER_NAME;
        i += (*((u_char *)(packet + i - 1)));
        i += OFFSET_BEFORE_CODE;

        // 代码长度
        int length = 0;
        length |= packet[i - 2] << 8;
        length |= packet[i - 1];
        printf("code length: %d bytes ---------\n", length);
        code = new char[length + 10];
        codeIndex = 0;
        memset(code, 0, sizeof(code));
        while (length > 0 && i < size) {
            code[codeIndex++] = packet[i];
            i++;
            length--;
        }
        return length;
    } else {
        return -1;
    }
}

const char filter[] = "port 5001 and src host 52.207.39.66";

int main() {
    PcapListener *plis = new PcapListener(filter);
    for (char cmd[100]; !plis->getInitFlag();) {
        std::cout << "reInit? [y/n]" << std::endl;
        std::cin >> cmd;
        if (tolower(cmd[0]) == 'y') {
            plis->init();
        } else {
            break;
        }
    }
    if (plis->getInitFlag()) {
        plis->runLoop(callBack);
    }
    delete plis;
}
