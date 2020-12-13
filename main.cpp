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
            auto cfg = rea::Json("width", 400,
                                 "height", 800,
                                 "arrow", rea::Json("visible", false,
                                                    "pole", true),
                                 "face", 200,
                                 "text", rea::Json("visible", false,
                                                   "size", rea::JArray(100, 50),
                                                   "location", "bottom"),
                                 "color", "blue",
                                 "objects", rea::Json(
                                                "shp_0", rea::Json(
                                                             "type", "poly",
                                                             "points", rea::JArray(QJsonArray(),
                                                                                   rea::JArray(50, 50, 200, 200, 200, 50, 50, 50),
                                                                                   rea::JArray(80, 70, 120, 100, 120, 70, 80, 70)),
                                                             "color", "red",
                                                             "width", 3,
                                                             "caption", "poly",
                                                             "face", 50,
                                                             "text", rea::Json("visible", true,
                                                                               "size", rea::JArray(100, 50))
                                                             ),
                                                "shp_1", rea::Json(
                                                             "type", "ellipse",
                                                             "center", rea::JArray(400, 400),
                                                             "radius", rea::JArray(300, 200),
                                                             "width", 5,
                                                             "ccw", false,
                                                             "angle", 30,
                                                             "caption", "ellipse"
                                                             )
                                                    ));

            rea::pipeline::run<QJsonObject>("updateQSGModel_frontend", cfg);
    }

};

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    aInput->data()->load(QUrl(QStringLiteral("qrc:/GUI/main.qml")));
    static document doc;
    aInput->out();
}, rea::Json("name", "loadMain"));
