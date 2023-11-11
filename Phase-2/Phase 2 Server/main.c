#include <stdio.h>
#include <winsock2.h>
#include "CJSON.h"
#include <conio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define SIZE 1000
#define PORT 12345

char *randstring(int length) {
    static int mySeed = 25011984;
    char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t stringLen = strlen(string);
    char *randomString = NULL;
    srand(time(NULL) * length + ++mySeed);
    if (length < 1) {
        length = 1;
    }
    randomString = malloc(sizeof(char) * (length + 1));
    if (randomString) {
        short key;
        for (int n = 0; n < length; n++) {
            key = rand() % stringLen;
            randomString[n] = string[key];
        }
        randomString[length] = '\0';
        return randomString;
    } else {
        printf("No memory");
        exit(1);
    }
}

char *nametok(char *token){
    char *username = malloc(20);
    FILE *user;
    char useraddr[60];

    sprintf(useraddr, "./res/user/%s.txt", token);
    user = fopen(useraddr, "r");
    if (user == NULL){
        return NULL;
    }
    fscanf(user, "%s", username);
    fclose(user);
    return username;
}

int FileCount(char *address){
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    int max = 0;
    dirp = opendir(address);
    while ((entry = readdir(dirp)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        sscanf(entry->d_name, "%d%*s", &max);
    }
    closedir(dirp);
    return max;
}

int initialize(int port);
void start(int server_socket);

void signup(char *message, char **response){
    char username[20], password[20];
    sscanf(message, "%*s %s %s", username, password);
    int len = strlen(username);
    username[len - 1] = '\0';
    char useraddr[40];
    sprintf(useraddr, "./res/user/%s.txt", username);
    cJSON *info = cJSON_CreateObject();
    cJSON *save = cJSON_CreateObject();
    FILE *user;
    user = fopen(useraddr, "r");
    if (user == NULL){
        cJSON_AddStringToObject(info, "type", "Successful");
        cJSON_AddStringToObject(info, "message", "");
        *response = cJSON_Print(info);

        fclose(user);
        user = fopen(useraddr, "w");
        cJSON_AddStringToObject(save, "username", username);
        cJSON_AddStringToObject(save, "password", password);
        cJSON_AddStringToObject(save, "bio", "");
        cJSON_AddArrayToObject(save, "followers");
        cJSON_AddArrayToObject(save, "followings");
        cJSON_AddArrayToObject(save, "personalTweets");
        cJSON_AddStringToObject(save, "token", "");
        cJSON_AddArrayToObject(save, "seen");

        char *file = cJSON_Print(save);
        fprintf(user, "%s", file);
    }else{
        cJSON_AddStringToObject(info, "type", "Error");
        cJSON_AddStringToObject(info, "message", "This username already exists");
        *response = cJSON_Print(info);
    }
    fclose(user);
}

void log_in(char *message, char **response){
    char username[20], password[20];
    sscanf(message, "%*s %s %s", username, password);
    int len = strlen(username);
    username[len - 1] = '\0';
    char useraddr[80];
    sprintf(useraddr, "./res/user/%s.txt", username);
    cJSON *res = cJSON_CreateObject();
    FILE *user = fopen(useraddr, "r");


    if (user == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "This user doesn't exists");
        *response = cJSON_Print(res);
        return;
    }
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *data = cJSON_Parse(info);
    cJSON *tok = cJSON_GetObjectItem(data, "token");

    if (strcmp(tok->valuestring, "") != 0){
        sprintf(useraddr, "./res/user/%s.txt", tok->valuestring);
        remove(useraddr);
        strcpy(tok->valuestring, "");
        sprintf(useraddr, "./res/user/%s.txt", username);
        user = fopen(useraddr, "w");
        char *save = cJSON_Print(data);
        fprintf(user,"%s", save);
        fclose(user);


        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "This user has already logged in. Try again.");
        *response = cJSON_Print(res);
        return;
    }


    cJSON *pass = cJSON_GetObjectItem(data, "password");
    //puts("4");
    if (strcmp(pass->valuestring, password) != 0){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Password is incorrect");
        *response = cJSON_Print(res);
        return;
    }
    char *token = malloc(40);
    token = randstring(32);
    sprintf(tok->valuestring, "%s", token);
    user = fopen(useraddr, "w");
    char *save = cJSON_Print(data);
    fprintf(user,"%s", save);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Token");
    cJSON_AddStringToObject(res, "message", tok->valuestring);
    *response = cJSON_Print(res);
    sprintf(useraddr, "./res/user/%s.txt", tok->valuestring);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", username);
    fclose(user);
    free(info);
    free(token);

}

void logout(char *message, char **response){
    char token[40], username[20];
    sscanf(message, "%*s %s", token);
    FILE *user;
    char useraddr[60];

    sprintf(useraddr, "./res/user/%s.txt", token);
    user = fopen(useraddr, "r");
    cJSON *res = cJSON_CreateObject();
    if (user == NULL){
        puts("4");
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "User isn't logged in");
        *response = cJSON_Print(res);
        return;
    }
    fscanf(user, "%s", username);
    fclose(user);
    remove(useraddr);
    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "r");
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    cJSON *save = cJSON_Parse(info);
    cJSON *tok = cJSON_GetObjectItem(save, "token");
    strcpy(tok->valuestring, "");
    char *info1 = cJSON_Print(save);
    fclose(user);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "");
    *response = cJSON_Print(res);
    free(info);
}

void setbio(char *message, char **response){
    char *username, token[40], bio[200], useraddr[100];
    sscanf(message, "%*s %*s %s %[^\n]s", token, bio);
    token[strlen(token) - 1] = '\0';
    cJSON *res = cJSON_CreateObject();
    username = nametok(token);
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", username);
    FILE *user;
    user = fopen(useraddr, "r");
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *save = cJSON_Parse(info);
    cJSON *bio1 = cJSON_GetObjectItem(save, "bio");
    if (strcmp(bio1->valuestring, "") != 0){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Bio is already set");
        *response = cJSON_Print(res);
        free(info);
        return;
    }
    sprintf(bio1->valuestring, "%s", bio);
    char *info1;
    info1 = cJSON_Print(save);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "Bio set successfully");
    *response = cJSON_Print(res);
    free(info);
    return;
}

void change_pass(char *message, char **response){
    char token[40], oldpass[20], newpass[20], useraddr[70], *username;
    sscanf(message, "%*s %*s %s %s %s", token, oldpass, newpass);
    token[strlen(token) - 1] = '\0';
    oldpass[strlen(oldpass) - 1] = '\0';
    cJSON *res = cJSON_CreateObject();
    username = nametok(token);
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", username);
    if (strlen(newpass) > 16){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Password can't be more than 16 characters");
        *response = cJSON_Print(res);
        return;
    }
    char *info = malloc(5000 * sizeof(char));
    FILE *user;
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *save = cJSON_Parse(info);
    cJSON *pass = cJSON_GetObjectItem(save, "password");
    if (strcmp(pass->valuestring, oldpass) != 0){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Password is incorrect");
        *response = cJSON_Print(res);
        free(info);
        return;
    }
    strcpy(pass->valuestring, newpass);
    info = cJSON_Print(save);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info);
    fclose(user);
    free(info);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "Password changed successfully");
    *response = cJSON_Print(res);
    return;

}

void send_tweet(char *message, char **response){
    int file = FileCount(".\\res\\tweets");
    file++;
    printf("%d", file);
    char tweet[4000], token[40], *username, useraddr[80];
    sscanf(message, "%*s %*s %s %[^\n]s", token, tweet);
    token[strlen(token) - 1] = '\0';
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", username);
    FILE *user;
    char *info = malloc(5000);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *save = cJSON_Parse(info);
    cJSON *personal = cJSON_GetObjectItem(save, "personalTweets");
    cJSON_AddItemToArray(personal, cJSON_CreateNumber(file));
    info = cJSON_Print(save);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "Tweet sent");
    *response = cJSON_Print(res);
    sprintf(useraddr, "./res/tweets/%d.txt", file);
    cJSON *tweetj = cJSON_CreateObject();
    cJSON_AddNumberToObject(tweetj, "id", file);
    cJSON_AddStringToObject(tweetj, "author", username);
    cJSON_AddStringToObject(tweetj, "content", tweet);
    cJSON *comments = cJSON_CreateArray();
    cJSON_AddItemToObject(tweetj, "comments", comments);
    cJSON_AddNumberToObject(tweetj, "likes", 0);
    info = cJSON_Print(tweetj);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info);
    fclose(user);
    free(info);
}

void profile(char *message, char **response){
    char *username, useraddr[80], token[40];
    sscanf(message, "%*s %s", token);
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", username);
    FILE *user;
    char *info = malloc(5000);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *save = cJSON_Parse(info);
    cJSON *name = cJSON_GetObjectItem(save, "username");
    cJSON *bio = cJSON_GetObjectItem(save, "bio");
    int numberOfFollowers = cJSON_GetArraySize(cJSON_GetObjectItem(save, "followers"));
    int numberOfFollowings = cJSON_GetArraySize(cJSON_GetObjectItem(save, "followings"));
    int numberOfTweets = cJSON_GetArraySize(cJSON_GetObjectItem(save, "personalTweets"));
    cJSON *arr = cJSON_GetObjectItem(save, "personalTweets");
    cJSON *allTweets = cJSON_CreateArray();

    for (int i = 0; i < numberOfTweets; i++){
        cJSON *num = cJSON_GetArrayItem(arr, i);
        int num1 = num->valueint;
        sprintf(useraddr, "./res/Tweets/%d.txt", num1);
        user = fopen(useraddr, "r");
        fscanf(user, "%[^\0]s", info);
        fclose(user);
        cJSON *temp = cJSON_Parse(info);
        cJSON_AddItemToArray(allTweets, temp);
    }
    cJSON_AddStringToObject(res, "type", "Profile");
    cJSON *messag = cJSON_CreateObject();
    cJSON_AddStringToObject(messag, name->string, name->valuestring);
    cJSON_AddStringToObject(messag, bio->string, bio->valuestring);
    cJSON_AddNumberToObject(messag, "numberOfFollowers", numberOfFollowers);
    cJSON_AddNumberToObject(messag, "numberOfFollowings", numberOfFollowings);
    cJSON_AddItemToObject(messag, "allTweets", allTweets);
    cJSON_AddItemToObject(res, "message", messag);
    *response = cJSON_Print(res);
    free(info);

}

void search(char *message, char **response){
    char *username, useraddr[80], token[40], search[30];
    sscanf(message, "%*s %s %s", token, search);
    token[strlen(token) - 1] = '\0';
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", search);
    FILE *user;
    char *info = malloc(5000);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *save = cJSON_Parse(info);
    cJSON *name = cJSON_GetObjectItem(save, "username");
    cJSON *bio = cJSON_GetObjectItem(save, "bio");
    int numberOfFollowers = cJSON_GetArraySize(cJSON_GetObjectItem(save, "followers"));
    int numberOfFollowings = cJSON_GetArraySize(cJSON_GetObjectItem(save, "followings"));
    int numberOfTweets = cJSON_GetArraySize(cJSON_GetObjectItem(save, "personalTweets"));
    cJSON *arr = cJSON_GetObjectItem(save, "personalTweets");
    cJSON *allTweets = cJSON_CreateArray();

    for (int i = 0; i < numberOfTweets; i++){
        cJSON *num = cJSON_GetArrayItem(arr, i);
        int num1 = num->valueint;
        sprintf(useraddr, "./res/Tweets/%d.txt", num1);
        user = fopen(useraddr, "r");
        fscanf(user, "%[^\0]s", info);
        fclose(user);
        cJSON *temp = cJSON_Parse(info);
        cJSON_AddItemToArray(allTweets, temp);
    }
    cJSON_AddStringToObject(res, "type", "Profile");
    cJSON *messag = cJSON_CreateObject();
    cJSON_AddStringToObject(messag, name->string, name->valuestring);
    cJSON_AddStringToObject(messag, bio->string, bio->valuestring);
    cJSON_AddNumberToObject(messag, "numberOfFollowers", numberOfFollowers);
    cJSON_AddNumberToObject(messag, "numberOfFollowings", numberOfFollowings);
    cJSON_AddItemToObject(messag, "allTweets", allTweets);

    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *check = cJSON_Parse(info);
    cJSON *array = cJSON_GetObjectItem(check, "followings");
    int num = cJSON_GetArraySize(array);
    int followed = 0;
    for (int i = 0; i < num; i++){
        cJSON *status = cJSON_GetArrayItem(array, i);
        if (strcmp(status->valuestring, search) == 0){
            cJSON_AddStringToObject(messag, "followStatus", "Followed");
            followed = 1;
            break;
        }
    }
    if (followed == 0){
        cJSON_AddStringToObject(messag, "followStatus", "notFollowed");
    }
    cJSON_AddItemToObject(res, "message", messag);
    *response = cJSON_Print(res);
    free(info);
}

void change_follow(char *message, char **response){
    char *username, token[40], search[40], useraddr[80];
    sscanf(message, "%*s %s %s", token, search);
    token[strlen(token) - 1] = '\0';
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", search);
    FILE *user;
    user = fopen(useraddr, "r");
    if (user == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "User doesn't exists");
        *response = cJSON_Print(res);
        return;
    }
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *user2 = cJSON_Parse(info);
    cJSON *followers = cJSON_GetObjectItem(user2, "followers");
    int num = cJSON_GetArraySize(followers);
    for (int i = 0; i < num; i++){
        cJSON *person = cJSON_GetArrayItem(followers, i);
        if (strcmp(person->valuestring, username) == 0){
            cJSON_AddStringToObject(res, "type", "Error");
            cJSON_AddStringToObject(res, "message", "You already follow this user");
            *response = cJSON_Print(res);
            free(info);
            return;
        }
    }
    cJSON_AddItemToArray(followers, cJSON_CreateString(username));
    char *info1 = cJSON_Print(user2);
    user  = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *user1 = cJSON_Parse(info);
    cJSON *followers1 = cJSON_GetObjectItem(user1, "followings");
    cJSON_AddItemToArray(followers1, cJSON_CreateString(search));
    info1 = cJSON_Print(user1);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "You are now following this user");
    *response = cJSON_Print(res);
    free(info);
}

void change_unfollow(char *message, char **response){
    char *username, token[40], search[40], useraddr[80];
    sscanf(message, "%*s %s %s", token, search);
    token[strlen(token) - 1] = '\0';
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(useraddr, "./res/user/%s.txt", search);
    FILE *user;
    user = fopen(useraddr, "r");
    if (user == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "User doesn't exists");
        *response = cJSON_Print(res);
        return;
    }

    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *user2 = cJSON_Parse(info);
    cJSON *followers = cJSON_GetObjectItem(user2, "followers");
    int num = cJSON_GetArraySize(followers);
    int follower = 0;
    int index;

    for (int i = 0; i < num; i++){
        cJSON *person = cJSON_GetArrayItem(followers, i);
        if (strcmp(person->valuestring, username) == 0){
            follower = 1;
            index = i;
            break;
        }
    }

    if (follower == 0){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "You don't follow this user");
        *response = cJSON_Print(res);
        free(info);
        return;
    }
    cJSON_DeleteItemFromArray(followers, index);
    char *info1 = cJSON_Print(user2);
    user  = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);

    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    printf("%s", info);
    cJSON *user1 = cJSON_Parse(info);
    cJSON *followers1 = cJSON_GetObjectItem(user1, "followings");
    num = cJSON_GetArraySize(followers1);
    for (int i = 0; i < num; i++){
        cJSON *person = cJSON_GetArrayItem(followers1, i);
        if (strcmp(person->valuestring, search) == 0){
            follower = 1;
            index = i;
            break;
        }
    }
    cJSON_DeleteItemFromArray(followers1, index);
    puts("3");
    info1 = cJSON_Print(user1);
    user = fopen(useraddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "You unfollowed user");
    *response = cJSON_Print(res);
    free(info);
}

void refresh(char *message, char **response){
    char *username, token[40], useraddr[80];
    sscanf(message, "%*s %s", token);
    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    cJSON *messag = cJSON_CreateArray();
    FILE *user;
    char *info = malloc(5000);
    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "r");
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    puts("123");

    cJSON *user1;
    user1 = cJSON_Parse(info);
    puts("2");
    cJSON *followings = cJSON_GetObjectItem(user1, "followings");
    puts("2");
    cJSON *seen = cJSON_GetObjectItem(user1, "seen");
    puts("2");
    int number = cJSON_GetArraySize(followings);

    for (int i = 0; i < number; i++){
        cJSON *person = cJSON_GetArrayItem(followings, i);
        sprintf(useraddr, "./res/user/%s.txt", person->valuestring);
        user = fopen(useraddr, "r");
        fscanf(user, "%[^\0]s", info);
        fclose(user);
        cJSON *useri = cJSON_Parse(info);
        cJSON *tweets = cJSON_GetObjectItem(useri, "personalTweets");
        int num = cJSON_GetArraySize(tweets);
        for (int j = 0; j < num; j++){
            cJSON *tweet_num = cJSON_GetArrayItem(tweets, j);
            int seen1 = cJSON_GetArraySize(seen);
            int same = 0;
            for (int k = 0; k < seen1; k++){
                cJSON *check = cJSON_GetArrayItem(seen, k);
                if (tweet_num->valueint == check->valueint){
                    same = 1;
                    break;
                }
            }
            printf("%d", same);
            if (same == 1){
                continue;
            }
            cJSON_AddItemToArray(seen, cJSON_CreateNumber(tweet_num->valueint));
            sprintf(useraddr, "./res/tweets/%d.txt", tweet_num->valueint);
            fopen(useraddr, "r");
            fscanf(user, "%[^\0]s", info);
            fclose(user);
            cJSON *temp = cJSON_Parse(info);
            cJSON_AddItemToArray(messag, temp);
        }
    }
    cJSON_AddStringToObject(res, "type", "List");
    cJSON_AddItemToObject(res, "message", messag);
    *response = cJSON_Print(res);
    sprintf(useraddr, "./res/user/%s.txt", username);
    user = fopen(useraddr, "w");
    char *info3 = cJSON_Print(user1);
    fprintf(user, "%s", info3);
    fclose(user);
    free(info);
}

void like(char *message, char **response){
    char *username, token[40], tweet_addr[80];
    int tweet_id;

    sscanf(message, "%*s %s %d", token, &tweet_id);

    token[strlen(token) - 1] = '\0';

    username = nametok(token);
    cJSON *res = cJSON_CreateObject();
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }

    sprintf(tweet_addr, "./res/tweets/%d.txt", tweet_id);
    FILE *user;
    user = fopen(tweet_addr, "r");
    if (user == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "ID is incorrect");
        *response = cJSON_Print(res);
        return;
    }
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    puts(info);
    cJSON *tweet = cJSON_Parse(info);
    cJSON *likes = cJSON_GetObjectItem(tweet, "likes");
    int num = likes->valueint;
    num++;
    cJSON_DeleteItemFromObject(tweet, "likes");
    cJSON_AddNumberToObject(tweet, "likes", num);

    info = cJSON_Print(tweet);
    puts(info);
    user = fopen(tweet_addr, "w");
    fprintf(user, "%s", info);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "You liked this tweet");
    *response = cJSON_Print(res);
    free(info);
}

void comment(char *message, char **response){
    char *username, token[40], tweetaddr[80], comment[200], id[20];
    int tweet_id;
    sscanf(message, "%*s %s %s %[^\n]s", token, id, comment);
    token[strlen(token) - 1] = '\0';
    id[strlen(id) - 1] = '\0';
    sscanf(id, "%d", &tweet_id);
    cJSON *res = cJSON_CreateObject();
    username = nametok(token);
    if (username == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "Token is not valid");
        *response = cJSON_Print(res);
        return;
    }
    sprintf(tweetaddr, "./res/tweets/%d.txt", tweet_id);
    FILE *user;
    user = fopen(tweetaddr, "r");
    if (user == NULL){
        cJSON_AddStringToObject(res, "type", "Error");
        cJSON_AddStringToObject(res, "message", "ID is incorrect");
        *response = cJSON_Print(res);
        return;
    }
    char *info = malloc(5000);
    fscanf(user, "%[^\0]s", info);
    fclose(user);
    cJSON *tweet = cJSON_Parse(info);
    cJSON *newComment = cJSON_CreateObject();
    cJSON_AddStringToObject(newComment, username, comment);
    cJSON *comments = cJSON_GetObjectItem(tweet, "comments");
    cJSON_AddItemToArray(comments, newComment);
    char *info1 = cJSON_Print(tweet);
    user = fopen(tweetaddr, "w");
    fprintf(user, "%s", info1);
    fclose(user);
    cJSON_AddStringToObject(res, "type", "Successful");
    cJSON_AddStringToObject(res, "message", "You commented on this tweet");
    *response = cJSON_Print(res);
    free(info);
}

int main(){
    mkdir(".\\res");
    mkdir(".\\res\\tweets");
    mkdir(".\\res\\user");
    int server_socket = initialize(PORT);
    if (server_socket != INVALID_SOCKET){
        start(server_socket);
    }
    return 0;
}

int initialize(int port){
    WSADATA wsadata;
    int wsa_res = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (wsa_res != 0){
        printf("Failed to run wsadata");
        return INVALID_SOCKET;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Could not create socket");
        return INVALID_SOCKET;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int bind_res = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bind_res == SOCKET_ERROR){
        printf("Bind Faild");
        return INVALID_SOCKET;
    }
    listen(server_socket, SOMAXCONN);
    return server_socket;
}

void start(int server_socket) {
    int len = sizeof(struct sockaddr_in);

    int client_socket;
    struct sockaddr_in client_addr;

    char *messages[SIZE];
    int size = 0;

    while((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &len))
          != INVALID_SOCKET) {
        printf("Client connected\n");
        char *buffer;
        buffer = (char *)malloc(SIZE * sizeof(char));
        memset(buffer, 0, SIZE);
        recv(client_socket, buffer, SIZE-1, 0);

        char *response;

        char type[20];
        sscanf(buffer, "%s", type);
        if (strcmp(type, "signup") == 0){
            signup(buffer, &response);
        }else if (strcmp(type, "login") == 0){
            log_in(buffer, &response);
        }else if (strcmp(type, "logout") == 0){
            logout(buffer, &response);
        }else if(strcmp(type, "set") == 0){
            setbio(buffer, &response);
        }else if(strcmp(type, "change") == 0){
            change_pass(buffer, &response);
        }else if(strcmp(type, "send") == 0){
            send_tweet(buffer, &response);
        }else if(strcmp(type, "profile") == 0){
            profile(buffer, &response);
        }else if(strcmp(type, "search") == 0){
            search(buffer, &response);
        }else if(strcmp(type, "follow") == 0){
            change_follow(buffer, &response);
        }else if(strcmp(type, "unfollow") == 0){
            change_unfollow(buffer, &response);
        }else if(strcmp(type, "like") == 0){
            like(buffer, &response);
        }else if(strcmp(type, "comment") == 0){
            comment(buffer, &response);
        }else if(strcmp(type, "refresh") == 0){
            refresh(buffer, &response);
        }

        printf("%s\n", response);

        send(client_socket, response, strlen(response), 0);
        closesocket(client_socket);

    }
}
