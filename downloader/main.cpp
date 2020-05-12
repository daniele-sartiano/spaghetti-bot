#include <iostream>
#include <fstream>
#include <pthread.h>
#include <curl/curl.h>

#include <hiredis/hiredis.h>
#include "src/QueueManager.h"
#include "src/KafkaReader.h"

#include <librdkafka/rdkafka.h>
#include <unistd.h>
#include <nats/nats.h>

/*// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
static size_t write_data_to_string(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::ofstream& outstream = *reinterpret_cast<std::ofstream*>(stream);
    outstream.write((const char*)ptr, size * nmemb);

    //size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return size*nmemb;
}*/


/*static void *pull_one_url(void *url)
{
    CURL *curl;

    std::string file_name = "page.";
    file_name.append((char*)url);
    file_name.append(".out");

    std::ofstream out_file;
    out_file.open(file_name);

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);

    curl_easy_perform(curl); *//* ignores error *//*

    out_file.close();

    curl_easy_cleanup(curl);

    return nullptr;
}*/


static void *redis_get_url() {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    auto *q = new RedisQueueManager(port, hostname, 1, 500000);
    q->get("domain2crawl");
}

/*void test_redis(redisContext *c) {

    redisReply *reply;
    *//* PING server *//*
    reply = static_cast<redisReply *>(redisCommand(c, "PING"));
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);


    *//* Set a key *//*
    reply = static_cast<redisReply *>(redisCommand(c, "SET %s %s", "foo", "hello world"));
    printf("SET: %s\n", reply->str);
    freeReplyObject(reply);

    *//* Try a GET and two INCR *//*
    reply = static_cast<redisReply *>(redisCommand(c, "GET foo"));
    printf("GET foo: %s\n", reply->str);
    freeReplyObject(reply);
}*/


void test_init_redis_queue() {
    const char *hostname = "127.0.0.1";
    int port = 6379;
    auto *q = new RedisQueueManager(port, hostname, 1, 500000);

    const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it"
    };

    for (const char *url: urls) {
        printf("---> %s\n", url);
        q->set("domain2crawl", url);
    }
}

void test_init_kafka_queue() {
    const char *hostname = "127.0.0.1:9092";
    auto *q = new KafkaQueueManager(hostname);

    const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it",
            "www.google.com",
            "www.unipi.it",
            "www.ebay.com"
    };

    for (const char *url: urls) {
        q->set("domain2crawl", url);
    }
}

static void *kafka_get_url(int i) {
    std::cout << "Thread " << i << std::endl;
    std::vector<std::string> topics;
    topics.emplace_back("domain2crawl");
    auto *kr = new KafkaReader("127.0.0.1:9092", "0", topics);
    int r = 0;
    while(true) {
        std::cout << "t " << i << " r is " << r << std::endl;
        for (RdKafka::Message *msg : kr->get()) {
            std::cout << "t " << i << " r is " << r << "data --> " << msg->payload() << std::endl;
        }
        sleep(1);
        r++;

    }
}

static void *download_url() {
    const char *hostname = "127.0.0.1:9092";
    auto *q = new KafkaQueueManager(hostname);
    q->get("domain2crawl");
}

int main(int argc, char **argv) {
    natsConnection      *conn = NULL;
    natsStatus          s;

    printf("Publishes a message on subject 'foo'\n");

    // Creates a connection to the default NATS URL
    s = natsConnection_ConnectTo(&conn, "nats://127.0.0.1:4222");
    if (s == NATS_OK)
    {
        const char data[] = {104, 101, 108, 108, 111, 33};

        // Publishes the sequence of bytes on subject "foo".
        s = natsConnection_Publish(conn, "foo", data, sizeof(data));
    }

    // Anything that is created need to be destroyed
    natsConnection_Destroy(conn);

    // If there was an error, print a stack trace and exit
    if (s != NATS_OK)
    {
        nats_PrintLastErrorStack(stderr);
        exit(2);
    }

    return 0;
}

int main3(int argc, char **argv) {
    int NUMT = 3;
    pthread_t tid[NUMT];
    int error;

    /*for (int n=0; n<100; n++) {
        create_producer();
    }*/

    /*std::vector<std::string> topics;
    topics.emplace_back("domain2crawl");
    auto *kr = new KafkaReader("127.0.0.1:9092", "0", topics);
    int r = 0;
    while(r < 10) {
        std::cout << " r is " << r << std::endl;
        for (RdKafka::Message *msg : kr->get()) {
            std::cout << "data --> " << msg->payload() << std::endl;
        }

        r++;
    }*/

    curl_global_init(CURL_GLOBAL_ALL);

    for (int i=0; i<NUMT; i++) {
        fprintf(stderr, "OK\n");
        error = pthread_create(
                &tid[i],
                nullptr,
                reinterpret_cast<void *(*)(void *)>(kafka_get_url),
                reinterpret_cast<void *>(i));
        std::cout << error << std::endl;
        if (0 != error) {
            fprintf(stderr, "Could't run thread number %d, errno %d\n", i, error);
        } else {
            //fprintf(stderr,  "Thread %d,  gets %s\n", i, urls[i]);
            fprintf(stderr,  "Thread %d started\n", i);
        }
    }

    /* now wait for all threads to terminate */
    for(int i=0; i< NUMT; i++) {
        error = pthread_join(tid[i], nullptr);
        fprintf(stderr, "Thread %d terminated\n", i);
    }

    return 0;

}

int main2(int argc, char **argv) {

    test_init_kafka_queue();

    //test_init_redis_queue();


    int NUMT = 3;
    pthread_t tid[NUMT];
    int error;

    /*const char * const urls[] = {
            "www.sartiano.info",
            "www.repubblica.it",
            "www.corriere.it"
    };*/


/*
    redisContext *c;

    const char *hostname = "127.0.0.1";
    int port = 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);

    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    test_redis(c);
*/

    curl_global_init(CURL_GLOBAL_ALL);

    /*for (int i=0; i<NUMT; i++) {
        fprintf(stderr, "OK");
        error = pthread_create(
                &tid[i],
                nullptr,
                reinterpret_cast<void *(*)(void *)>(kafka_get_url(i)),
                nullptr);
        if (0 != error) {
            fprintf(stderr, "Could't run thread number %d, errno %d\n", i, error);
        } else {
            //fprintf(stderr,  "Thread %d,  gets %s\n", i, urls[i]);
            fprintf(stderr,  "Thread %d started\n", i);
        }
    }*/

    /* now wait for all threads to terminate */
    for(int i=0; i< NUMT; i++) {
        error = pthread_join(tid[i], nullptr);
        fprintf(stderr, "Thread %d terminated\n", i);
    }

    return 0;
}