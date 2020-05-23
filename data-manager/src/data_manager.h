//
// Created by lele on 21/05/20.
//

#ifndef DATA_MANAGER_DATA_MANAGER_H
#define DATA_MANAGER_DATA_MANAGER_H


#include <string>
#include <cassandra.h>
#include <model.h>

class DataManager {
private:
    CassSession* session;
    CassError connect_session(CassSession* session, const CassCluster* cluster);
    static void log_error(CassFuture* future);
    std::string db = "penelopebot";
    std::string table = "data";
public:
    explicit DataManager(const std::string& hosts, const std::string& dbname = "", const std::string& tablename = "") {
        CassCluster* cluster = nullptr;
        this->session = cass_session_new();

        cluster = cass_cluster_new();
        cass_cluster_set_contact_points(cluster, hosts.c_str());

        if (connect_session(this->session, cluster) != CASS_OK) {
            cass_cluster_free(cluster);
            cass_session_free(this->session);
            exit(EXIT_FAILURE);
        }

        if (!dbname.empty()) {
            this->db = dbname;
        }

        std::string q = "CREATE KEYSPACE IF NOT EXISTS ";
        q.append(this->db);
        q.append(" WITH replication = { 'class': 'SimpleStrategy', 'replication_factor': '3' };");
        this->query(q);

        if (!tablename.empty()) {
            this->table = tablename;
        }
        q = "CREATE TABLE IF NOT EXISTS ";
        q.append(this->db);
        q.append(".");
        q.append(this->table);
        q.append(" (timestamp bigint, link text, text text, PRIMARY KEY (timestamp, link));");

        this->query(q);
    }

    CassError query(const std::string& query);
    CassError insert_model(const Model& model);
};


#endif //DATA_MANAGER_DATA_MANAGER_H