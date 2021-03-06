//
// Created by lele on 22/05/20.
//

#include "model.h"

std::string Model::serialize() {

    StringBuffer sb = nullptr;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();

    for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME, Model::KEY_IP,
                         Model::KEY_LINKS}) {
        writer.Key(k.c_str());
        if (k == Model::KEY_TIMESTAMP) {
            writer.Int64(this->timestamp);
        } else if (k == Model::KEY_LINK) {
            writer.String(this->link.c_str());
        } else if (k == Model::KEY_TEXT) {
            writer.String(this->text.c_str());
        } else if (k == Model::KEY_FILENAME) {
            writer.String(this->filename.c_str());
        } else if (k == Model::KEY_IP) {
            writer.String(this->ip.c_str());
        } else if (k == Model::KEY_LINKS) {
            writer.StartArray();
            for (const std::string &s : this->links) {
                writer.String(s.c_str());
            }
            writer.EndArray();
        }
    }

    writer.EndObject();
    return sb.GetString();
}

void Model::set_timestamp(long timestamp) {
    Model::timestamp = timestamp;
}

void Model::set_link(const std::string &link) {
    Model::link = link;
}

void Model::set_text(const std::string &text) {
    Model::text = text;
}

void Model::set_filename(const std::string &filename) {
    Model::filename = filename;
}

void Model::set_ip(const std::string &ip) {
    Model::ip = ip;
}

void Model::set_links(const std::set<std::string> &links) {
    Model::links = links;
}

std::vector<Model> Model::deserialize_models(const std::string &serialized) {
    std::vector<Model> models;

    Document d;
    Value v_models;
    Value v;
    d.Parse(serialized.c_str());

    if (d.HasMember(Model::KEY_MODELS)) {
        v_models = d[Model::KEY_MODELS];
        if (v_models.IsArray()) {
            for (SizeType i = 0; i < v_models.Size(); i++) {

                Value o = v_models[i].GetObject();
                int timestamp;
                std::string link;
                std::string text;
                std::string filename;
                std::string ip;
                std::set<std::string> links;

                for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME,
                                     Model::KEY_IP, Model::KEY_LINKS}) {
                    if (o.HasMember(k.c_str())) {
                        v = o[k.c_str()];
                        if (k == Model::KEY_TIMESTAMP) {
                            timestamp = v.GetInt64();
                        } else if (k == Model::KEY_LINK) {
                            link = v.GetString();
                        } else if (k == Model::KEY_TEXT) {
                            text = v.GetString();
                        } else if (k == Model::KEY_FILENAME) {
                            filename = v.GetString();
                        } else if (k == Model::KEY_IP) {
                            ip = v.GetString();
                        } else if (k == Model::KEY_LINKS && v.IsArray()) {
                            for (SizeType i = 0; i < v.Size(); i++) {
                                links.insert(v[i].GetString());
                            }
                        }
                    }
                }
                models.emplace_back(Model(timestamp, link, text, filename, ip, links));
            }
        }
    }
    return models;
}

std::string Model::serialize_models(std::vector<Model> models) {
    StringBuffer sb = nullptr;
    Writer<StringBuffer> writer(sb);
    writer.StartObject();
    writer.Key(Model::KEY_MODELS);
    writer.StartArray();
    for (auto m: models) {
        writer.StartObject();

        for (std::string k: {Model::KEY_TIMESTAMP, Model::KEY_LINK, Model::KEY_TEXT, Model::KEY_FILENAME, Model::KEY_IP,
                             Model::KEY_LINKS}) {
            writer.Key(k.c_str());
            if (k == Model::KEY_TIMESTAMP) {
                writer.Int64(m.get_timestamp());
            } else if (k == Model::KEY_LINK) {
                writer.String(m.get_link().c_str());
            } else if (k == Model::KEY_TEXT) {
                writer.String(m.get_text().c_str());
            } else if (k == Model::KEY_FILENAME) {
                writer.String(m.get_filename().c_str());
            } else if (k == Model::KEY_IP) {
                writer.String(m.get_ip().c_str());
            } else if (k == Model::KEY_LINKS) {
                writer.StartArray();
                for (const std::string &s : m.get_links()) {
                    writer.String(s.c_str());
                }
                writer.EndArray();
            }
        }
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}
