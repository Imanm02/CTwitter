#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include <winsock2.h>

struct sockaddr_in server;
int m_index = 0;
char token[50];
int login = 0;
typedef struct{
    char type[10];
    char message[100];
}Struct;

void log_in();
void sign_up();
void main_menu();
int initialize();
char *send_data(char *);
void print_new();
void menu();
void Timeline();
void send_tweet();
void refresh();
void like();
void comment();
void Search();
void tweet_profile();
void personal_area();
void set_bio();
void change_pass();
void log_out();
int main() {
    if (initialize(12345) == 0){
        while(1) {
            main_menu();
            while (login == 1) {
                menu();
            }
        }
    }
    return 0;
}

int initialize(int port) {
    WSADATA wsadata;
    int wsaStartUp = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsaStartUp != 0) {
    printf("Error\n");
    return -1;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(port);
    return 0;
}

char *send_data(char * data){
    char * buffer = malloc(1000);
    memset(buffer, 0, 1000);
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == INVALID_SOCKET){
        return "error1";
    }
    int can_connect = connect(client_socket, (struct sockaddr *) &server, sizeof(server));
    if (can_connect != 0){
        return "error2";
    }
    send(client_socket, data, strlen(data), 0);
    recv(client_socket, buffer, 999, 0);
    closesocket(client_socket);
    return buffer;
}

void print_new(){
    while(1){
        char *message;
        char data[15];
        sprintf(data, ":%d", m_index);
        message = send_data(data);
        if (message[0] == ':') {
            break;
        }
        printf("%s\n", message);
        m_index++;
    }
}

void main_menu(){
    system("cls");
    printf("Welcome to twitter\n");
    printf("1. log in\n2. sign up\n");
    int num;
    fflush(stdin);
    scanf("%d", &num);
    switch (num) {
        case 1:
            log_in();
            break;
        case 2:
            sign_up();
            break;
        default:
            puts("Wrong input");
            fflush(stdin);
            getchar();
    }
}

void log_in(){
    system("cls");
    printf("Enter your username\n");
    char name[100];
    fflush(stdin);
    scanf("%s", name);
    printf("Enter your password\n");
    char password[100];
    fflush(stdin);
    scanf("%s", password);
    char data[200];
    sprintf(data, "login %s, %s\n", name, password);
    char *response = send_data(data);
    cJSON *message = cJSON_Parse(response);
    cJSON *type = cJSON_GetObjectItem(message, "type");
    cJSON *res =cJSON_GetObjectItem(message, "message");
    if (strcmp(type->valuestring, "Token") != 0){
        puts(res->valuestring);
    } else{
        strcpy(token, res->valuestring);
        puts("\nLogged in successfully");
        login = 1;
    }
    fflush(stdin);
    getchar();
}

void sign_up(){
    system("cls");
    printf("1. I already have an account\n2. continue");
    int a;
    scanf("%d", &a);
    if (a == 1){
        return;
    }
    printf("Set a username\n");
    char name[100];
    fflush(stdin);
    scanf("%s", name);
    printf("Set a password\n");
    char password[100];
    fflush(stdin);
    scanf("%s", password);
    char send[100];
    sprintf(send, "signup %s, %s\n", name, password);
    char *server_response = send_data(send);
    char server_response1[100];
    printf("\n");
    char *tok;
    tok = strtok(server_response,"\"");
    int count = 1;
    while(tok != NULL){
        if (count == 4 ) {
            printf("%s\n", tok);
            if(strcmp(tok, "Successful") == 0){
                break;
            }
        }
        if (count == 8 ){
            puts(tok);
        }
        count++;
        tok = strtok(NULL, "\"");
    }
    fflush(stdin);
    getchar();
}

void menu(){
    system("cls");
    printf("1. Timeline\n");
    printf("2. Search\n");
    printf("3. Tweet profile\n");
    printf("4. Personal area\n");
    printf("5. Log out\n");
    int a;
    scanf("%d", &a);
    switch (a) {
        case 1:
            Timeline();
            break;
        case 2:
            Search();
            break;
        case 3:
            tweet_profile();
            break;
        case 4:
            personal_area();
            break;
        case 5:
            log_out();
            break;
        default:
            puts("Wrong input");
            fflush(stdin);
            getchar();
    }
}

void Timeline(){
    while(1) {
        system("cls");
        printf("1. Send Tweet\n");
        printf("2. Refresh\n");
        printf("3. Return\n");
        int num;
        scanf("%d", &num);
        switch (num) {
            case 1:
                send_tweet();
                break;
            case 2:
                refresh();
                break;
            case 3:
                return;
            default:
                puts("Wrong input");
                fflush(stdin);
                getchar();
        }
    }
}

void send_tweet(){
    system("cls");
    char tweet[400];
    printf("Insert your tweet\n");
    fflush(stdin);
    scanf("%[^\n]s", tweet);
    char data[500];
    sprintf(data,"send tweet %s, %s\n", token, tweet);
    send_data(data);
    puts("--------------------");
    printf("Tweet sent\n");
    fflush(stdin);
    getchar();
}

void refresh(){
    system("cls");
    char data[100];
    sprintf(data, "refresh %s\n", token);
    char *response = send_data(data);

    cJSON *message = cJSON_Parse(response);

    cJSON *array = cJSON_GetObjectItem(message, "message");

    int count = cJSON_GetArraySize(array);
    for (int i = 0; i < count; i++){
        cJSON *array2 = cJSON_GetArrayItem(array, i);
        cJSON *id = cJSON_GetObjectItem(array2, "id");
        cJSON *author = cJSON_GetObjectItem(array2, "author");
        cJSON *content = cJSON_GetObjectItem(array2, "content");
        cJSON *comments = cJSON_GetObjectItem(array2, "comments");
        cJSON *likes = cJSON_GetObjectItem(array2, "likes");
        int num = cJSON_GetArraySize(comments);
        printf("ID : %d\t", id->valueint);
        printf("Author : %s\n", author->valuestring);
        printf("%s\n", content->valuestring);
        printf("likes : %d\tComments : %d\n--------------------\n", likes->valueint, num);

    }
    while(1) {
        printf("1. like\n2. comment\n3. show comments\n4. Return\n");
        int a;
        scanf("%d", &a);
        switch (a) {
            case 1:
                like();
                break;
            case 2:
                comment();
                break;
            case 3:
                puts("Enter id of the tweet");
                int num_id;
                scanf("%d", &num_id);
                for (int i = 0; i < count; i++) {
                    cJSON *array3 = cJSON_GetArrayItem(array, i);
                    cJSON *id2 = cJSON_GetObjectItem(array3, "id");
                    if (id2->valueint == num_id) {
                        cJSON *comment1 = cJSON_GetObjectItem(array3, "comments");
                        int comment_num = cJSON_GetArraySize(comment1);
                        for (int j = 0; j < comment_num; j++) {
                            cJSON *member = cJSON_GetArrayItem(comment1, j);
                            printf("%s : %s\n--------------------\n", member->string, member->valuestring);
                        }
                    }
                }
                break;
            case 4:
                return;
            default:
                puts("Wrong input");
        }
    }
    fflush(stdin);
    getchar();
}

void like(){
    puts("Enter id of the tweet");
    char id[20];
    scanf("%s", id);
    char data[100];
    sprintf(data, "like %s, %s\n", token, id);
    char *response = send_data(data);
    cJSON *message = cJSON_Parse(response);
    cJSON *res = cJSON_GetObjectItem(message, "message");
    puts(res->valuestring);
    printf("--------------------\n");

}

void comment(){
    puts("Enter id of the tweet");
    char id[20];
    scanf("%s", id);
    puts("Enter your comment");
    char comment[200];
    fflush(stdin);
    scanf("%[^\n]s", comment);
    char data[300];
    sprintf(data, "comment %s, %s, %s\n", token, id, comment);
    char *response = send_data(data);
    cJSON *message = cJSON_Parse(response);
    cJSON *res = cJSON_GetObjectItem(message, "message");
    puts(res->valuestring);
    printf("--------------------\n");
}

void Search(){
    system("cls");
    printf("Enter the username you are looking for:\n");
    char name[50];
    scanf("%s", name);
    char data[200];
    sprintf(data, "search %s, %s\n", token, name);
    char *response = send_data(data);
    cJSON *server_response = cJSON_Parse(response);
    cJSON *type = cJSON_GetObjectItem(server_response, "type");
    cJSON *message = cJSON_GetObjectItem(server_response, "message");
    if (strcmp(type->valuestring, "Error") == 0){
        puts(message->valuestring);
        fflush(stdin);
        getchar();
        return;
    }
    cJSON *username =cJSON_GetObjectItem(message, "username");
    cJSON *bio =cJSON_GetObjectItem(message, "bio");
    cJSON *status = cJSON_GetObjectItem(message, "followStatus");
    cJSON *Followers =cJSON_GetObjectItem(message, "numberOfFollowers");
    cJSON *Followings =cJSON_GetObjectItem(message, "numberOfFollowings");
    cJSON *array = cJSON_GetObjectItem(message, "allTweets");
    int count = cJSON_GetArraySize(array);
    printf("\nusername : %s\n", username->valuestring);
    printf("bio : %s\n", bio->valuestring);
    printf("Followers : %d\t", Followers->valueint);
    printf("Followings : %d\n", Followings->valueint);
    printf("%s\n\n", status->valuestring);
    for (int i = 0; i < count;i++){
        cJSON *array2 = cJSON_GetArrayItem(array, i);
        cJSON *id = cJSON_GetObjectItem(array2, "id");
        cJSON *content = cJSON_GetObjectItem(array2, "content");
        cJSON *comments = cJSON_GetObjectItem(array2, "comments");
        cJSON *likes = cJSON_GetObjectItem(array2, "likes");
        int num = cJSON_GetArraySize(comments);
        printf("id : %d\n", id->valueint);
        puts(content->valuestring);
        printf("Likes : %d\tComments : %d\n--------------------\n", likes->valueint, num);
    }
    puts("Enter \"follow\" or \"unfollow\" if you wish to follow\\unfollow this user");
    char follow[20];
    fflush(stdin);
    scanf("%s", follow);
    if (strcmp(follow, "follow") == 0){
        char data1[200];
        sprintf(data1, "follow %s, %s\n", token, name);
        send_data(data1);
        printf("You are now following %s", name);
    }else if(strcmp(follow, "unfollow") == 0){
        char data1[200];
        sprintf(data1, "unfollow %s, %s\n", token, name);
        send_data(data1);
        printf("You unfollowed %s", name);
    }
    fflush(stdin);
    getchar();
}

void tweet_profile(){
    system("cls");
    char data[200];
    sprintf(data, "profile %s\n", token);
    char *response = send_data(data);
    cJSON *server_response = cJSON_Parse(response);
    cJSON *message = cJSON_GetObjectItem(server_response, "message");
    cJSON *username =cJSON_GetObjectItem(message, "username");
    cJSON *bio =cJSON_GetObjectItem(message, "bio");
    cJSON *Followers =cJSON_GetObjectItem(message, "numberOfFollowers");
    cJSON *Followings =cJSON_GetObjectItem(message, "numberOfFollowings");
    cJSON *array = cJSON_GetObjectItem(message, "allTweets");
    int count = cJSON_GetArraySize(array);
    printf("username : %s\n", username->valuestring);
    printf("bio : %s\n", bio->valuestring);
    printf("Followers : %d\t", Followers->valueint);
    printf("Followings : %d\n\n", Followings->valueint);
    for (int i = 0; i < count;i++){
        cJSON *array2 = cJSON_GetArrayItem(array, i);
        cJSON *id = cJSON_GetObjectItem(array2, "id");
        cJSON *content = cJSON_GetObjectItem(array2, "content");
        cJSON *comments = cJSON_GetObjectItem(array2, "comments");
        cJSON *likes = cJSON_GetObjectItem(array2, "likes");
        int num = cJSON_GetArraySize(comments);
        printf("id : %d\n", id->valueint);
        puts(content->valuestring);
        printf("Likes : %d\tComments : %d\n--------------------\n", likes->valueint, num);
    }
    fflush(stdin);
    getchar();
}

void personal_area(){
    while(1) {
        system("cls");
        printf("1. set bio\n");
        printf("2. change password\n");
        printf("3. Return\n");
        int num;
        scanf("%d", &num);
        switch (num) {
            case 1:
                set_bio();
                break;
            case 2:
                change_pass();
                break;
            case 3:
                return;
            default:
                puts("Wrong input");
                fflush(stdin);
                getchar();
        }
    }
}

void set_bio(){
    system("cls");
        printf("Please set your bio\n");
        char bio[200];
        fflush(stdin);
        scanf("%[^\n]s", bio);
        char data[300];
        sprintf(data, "set bio %s, %s\n",token, bio);
        char *response = cJSON_Parse(send_data(data));
        puts(response);
        Struct cjson;
        sprintf(cjson.type ,"%s", cJSON_GetStringValue
        (cJSON_GetObjectItemCaseSensitive(response , "type")));
        sprintf(cjson.message ,"%s", cJSON_GetStringValue
        (cJSON_GetObjectItemCaseSensitive(response , "message")));
        puts(cjson.type);
        puts(cjson.message);
        fflush(stdin);
        getchar();
}

void change_pass(){
    system("cls");
    printf("Enter your current password\n");
    char old_password[100];
    fflush(stdin);
    scanf("%s", old_password);
    printf("Enter your new password\n");
    char new_password[100];
    fflush(stdin);
    scanf("%s", new_password);
    char data[100];
    sprintf(data,"change password %s, %s, %s\n", token, old_password, new_password);
    char *response = send_data(data);
    char *tok;
    tok = strtok(response, "\"");
    int count = 1;
    while (tok != NULL){
        if( count == 4 || count == 8){
            puts(tok);
        }
        count++;
        tok = strtok(NULL, "\"");
    }
    fflush(stdin);
    getchar();
}

void log_out(){
    login = 0;
    char data[100];
    sprintf(data, "logout %s\n", token);
    send_data(data);
    printf("Goodbye\n");
    fflush(stdin);
    getchar();
}