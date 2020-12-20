#include "reactive2.h"
#include "imagePool.h"

class eObject;

class eLink : public QJsonObject{
private:
    eObject* m_start;
    eObject* m_end;
};

class eObject : public QJsonObject{
private:
    std::vector<eLink*> m_links;
};

class document{
public:
    document(){        
        const auto param = rea::Json("com1", rea::Json("param1_1", 1,
                                                       "param1_2", true),
                                     "com2", rea::Json("param2_1", "param",
                                                       "param2_2", rea::Json("use", true)),
                                     "com3", rea::Json("param3_1", false),
                                     "com4", rea::Json("param4_1", "value4_1",
                                                       "param4_2", 34,
                                                       "param4_3", false),
                                     "image", rea::Json("name", "image0",
                                                        "position", QJsonArray(),
                                                        "comment", "",
                                                        "relative_position", QJsonArray(),
                                                        "source", ""),
                                     "text", rea::Json("name", "text0",
                                                       "position", QJsonArray(),
                                                       "comment", "",
                                                       "relative_position", QJsonArray(),
                                                       "content", "",
                                                       "size", 16,
                                                       "color", "green",
                                                       "bold", ""),
                                     "shape", rea::Json("name", "text0",
                                                        "position", QJsonArray(),
                                                        "comment", "",
                                                        "relative_position", QJsonArray(),
                                                        "direction", rea::Json(
                                                            "color", "green",
                                                            "bolder", rea::Json(
                                                                "type", "line",
                                                                "color", "red"
                                                                ),
                                                            "radius", 30
                                                            )));
        const auto shape = rea::Json(
                    "image", rea::Json("type", "poly",
                                       "points", rea::JArray(QJsonArray(),
                                                             rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                       "color", "blue",
                                       "face", "50",
                                       "caption", "image"),
                    "text", rea::Json("type", "poly",
                                      "points", rea::JArray(QJsonArray(),
                                                            rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                      "color", "blue",
                                      "face", "50",
                                      "caption", "text"),
                    "shape", rea::Json("type", "poly",
                                      "points", rea::JArray(QJsonArray(),
                                                            rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                      "color", "blue",
                                      "face", "50",
                                      "caption", "shape")
                    );

        auto cfg = rea::Json("width", 400,
                             "height", 800,
                             "text", rea::Json("visible", true,
                                               "size", rea::JArray(40, 20),
                                               "location", "middle"));

        rea::pipeline::run<QJsonObject>("updateQSGModel_frontend", cfg);
        rea::pipeline::run<QJsonObject>("updateQSGModel_backend", cfg);
        rea::pipeline::run<QJsonObject>("updateQSGModel_elementend", cfg);

        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                             rea::Json(
                                                                 "type", "poly",
                                                                 "points", rea::JArray(QJsonArray(),
                                                                                       rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                                                 "color", "red",
                                                                 "face", "50",
                                                                 "caption", aInput->data()
                                                                 )
                                                             )), "frontend_pasteShapes");
        }, rea::Json("name", "createFrontEndCom"));

        rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
            aInput->out<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                             rea::Json(
                                                                 "type", "poly",
                                                                 "points", rea::JArray(QJsonArray(),
                                                                                       rea::JArray(0, 25, 50, 50, 100, 25, 50, 0, 0, 25)),
                                                                 "color", "blue",
                                                                 "face", "50",
                                                                 "caption", aInput->data()
                                                                                                                                      )
                                                             )), "backend_pasteShapes");
        }, rea::Json("name", "createBackEndCom"));

        rea::pipeline::add<QString>([shape](rea::stream<QString>* aInput){
            aInput->out<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                             shape.value(aInput->data()).toObject()
                                                             )), "elementend_pasteShapes");
        }, rea::Json("name", "createElementEndCom"));

        auto show_prm = [param](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto shps = dt.value("shapes").toObject();
            QJsonObject prm;
            if (shps.size() > 0){
                auto key = shps.begin().value().toObject().value("caption").toString();
                prm = param.value(key).toObject();
            }
            aInput->out<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
        };
        rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_frontend"));
        rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_backend"));
        rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_elementend"));
    }
private:
    QJsonObject m_user_data;
};

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    aInput->data()->load(QUrl(QStringLiteral("qrc:/GUI/main.qml")));
    static document doc;
    aInput->out();
}, rea::Json("name", "loadMain"));
