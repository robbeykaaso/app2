#include "main.h"
#include "storage0.h"

QJsonObject comObject::generateDocument(){
    auto ret = QJsonObject(*this);
    QJsonArray clds;
    for (auto i : m_children)
        clds.push_back(i->generateDocument());
    ret.insert("children", clds);
    return ret;
}

void document::comManagement(){
    m_root_com = std::make_shared<projectObject>("root", this);
    m_sel_com = m_root_com.get();

    //save com
    rea::pipeline::find("_selectFile")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            auto pth = dt[0].toString();
            if (!pth.endsWith(".json"))
                pth += ".json";
            aInput->out<rea::stgJson>(rea::stgJson(m_root_com->generateDocument(), pth), "writeJson");
        }, rea::Json("tag", "saveComModel"));

    //select com
    rea::pipeline::find("_treeViewSelected")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            m_sel_com = m_coms.value(aInput->data());
            if (!m_sel_com)
                m_sel_com = m_root_com.get();
        }, rea::Json("tag", "manual"));

    //new folder
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (!m_sel_com)
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "no parent com!"), "popMessage");
            else if (m_sel_com->getType() == "project" || m_sel_com->getType() == "folder")
                m_sel_com->addChild(std::make_shared<folderObject>(dt.value("name").toString(), m_sel_com->getParent()));
            else
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "create failed!"), "popMessage");
            aInput->out<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
        }, rea::Json("tag", "newFolder"));

    //new page
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (!m_sel_com)
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "no parent com!"), "popMessage");
            else if (m_sel_com->getType() == "project" || m_sel_com->getType() == "folder")
                m_sel_com->addChild(std::make_shared<pageObject>(dt.value("name").toString(), m_sel_com->getParent()));
            else
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "create failed!"), "popMessage");
            aInput->out<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
        }, rea::Json("tag", "newPage"));

    rea::pipeline::run<QJsonObject>("_updateTreeView", rea::Json("data", m_root_com->generateDocument(),
                                                                 "select", m_root_com->getID()));
}

document::document(){
    static rea::fsStorage0 stg;
    comManagement();
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


comObject::comObject(const QString& aName, document* aParent){
    m_parent = aParent;
    insert("name", aName);
    auto id = rea::generateUUID();
    insert("id", id);
    aParent->m_coms.insert(id, this);
}

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    aInput->data()->load(QUrl(QStringLiteral("qrc:/GUI/main.qml")));
    static document doc;
    aInput->out();
}, rea::Json("name", "loadMain"));
