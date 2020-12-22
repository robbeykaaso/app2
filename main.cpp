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

void comObject::removeChild(const QString& aID){
    for (auto i = 0; i < m_children.size(); ++i)
        if (m_children[i]->getID() == aID){
            m_parent->m_coms.remove(aID);
            m_children.erase(m_children.begin() + i);
            break;
        }
}

QJsonObject document::preparePageView(const QJsonObject& aPageConfig){
    QJsonObject ret;
    auto clds = aPageConfig.value("children").toArray();
    for (auto i : clds){
        auto cld = i.toObject();
        ret.insert(cld.value("id").toString(), rea::Json(m_shape_template.value(cld.value("type").toString()).toObject(),
                                                         "points", cld.value("points").toArray()));
    }
    return ret;
}

void document::comManagement(){
    m_root_com = std::make_shared<projectObject>("root", this);
    m_sel_com = m_root_com.get();

    //save document
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
            if (m_sel_com->getType() == "page"){
                aInput->out<QJsonArray>(rea::JArray(rea::Json("type", "select")), "updateQSGCtrl_elementend");
                aInput->out<QJsonObject>(rea::Json(m_page_template, "objects", preparePageView(m_sel_com->generateDocument())), "updateQSGModel_elementend");
            }
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

    //new com
    rea::pipeline::find("QSGAttrUpdated_elementend")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (m_sel_com->getType() == "page"){
                bool ch = false;
                for (auto i : dt){
                    auto mdy = i.toObject();
                    if (mdy.value("cmd").toBool()){
                        if (mdy.value("type") == "add"){
                            auto obj = mdy.value("val").toObject();
                            if (obj.value("com_type") == "image")
                                m_sel_com->addChild(std::make_shared<imageObject>("image", this, mdy.value("tar").toString()));
                            else if (obj.value("com_type") == "shape")
                                m_sel_com->addChild(std::make_shared<shapeObject>("shape", this, mdy.value("tar").toString()));
                            else if (obj.value("com_type") == "text")
                                m_sel_com->addChild(std::make_shared<textObject>("text", this, mdy.value("tar").toString()));
                            ch = true;
                        }
                        else if (mdy.value("type") == "del"){
                            m_sel_com->removeChild(mdy.value("tar").toString());
                            ch = true;
                        }
                        else{
                            auto com = m_coms.value(mdy.value("obj").toString());
                            if (com){
                                auto key = mdy.value("key").toArray()[0].toString();
                                com->insert(key, mdy.value("val"));
                            }
                        }
                    }
                }
                if (ch)
                    aInput->out<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
            }
        });

    auto sel = m_root_com->getID();

    auto test = std::make_shared<pageObject>("page0", this);
    m_root_com->addChild(test);
    m_sel_com = test.get();
    sel = test->getID();

    rea::pipeline::run<QJsonObject>("_updateTreeView", rea::Json("data", m_root_com->generateDocument(),
                                                                 "select", sel));
}

document::document(){
    m_page_template = rea::Json("width", 400,
                                "height", 800,
                                "text", rea::Json("visible", true,
                                                  "size", rea::JArray(40, 20),
                                                  "location", "middle"));

    static rea::fsStorage0 stg;
    comManagement();
    m_param_template = rea::Json("com1", rea::Json("param1_1", 1,
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
                                                    "source", ""),
                                 "text", rea::Json("name", "text0",
                                                   "position", QJsonArray(),
                                                   "comment", "",
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
    m_shape_template = rea::Json(
                "image", rea::Json("type", "poly",
                                   "com_type", "image",
                                   "points", rea::JArray(QJsonArray(),
                                                         rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                   "color", "blue",
                                   "face", "50",
                                   "caption", "image"),
                "text", rea::Json("type", "poly",
                                  "com_type", "text",
                                  "points", rea::JArray(QJsonArray(),
                                                        rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                  "color", "blue",
                                  "face", "50",
                                  "caption", "text"),
                "shape", rea::Json("type", "poly",
                                   "com_type", "shape",
                                   "points", rea::JArray(QJsonArray(),
                                                         rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                   "color", "blue",
                                   "face", "50",
                                   "caption", "shape")
                );

    rea::pipeline::run<QJsonObject>("updateQSGModel_frontend", m_page_template);
    rea::pipeline::run<QJsonObject>("updateQSGModel_backend", m_page_template);
    rea::pipeline::run<QJsonObject>("updateQSGModel_elementend", m_page_template);

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

    rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
        aInput->out<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                         m_shape_template.value(aInput->data()).toObject()
                                                         )), "elementend_pasteShapes");
    }, rea::Json("name", "createElementEndCom"));

    auto show_prm = [this](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto shps = dt.value("shapes").toObject();
        QJsonObject prm;
        if (shps.size() > 0){
            prm = QJsonObject(*m_coms.value(shps.keys()[0]));
            prm.remove("points");
            prm.remove("id");
            prm.remove("type");
        }
        aInput->out<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
    };
    rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_frontend"));
    rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_backend"));
    rea::pipeline::add<QJsonObject>(show_prm, rea::Json("name", "updateQSGSelects_elementend"));
}


comObject::comObject(const QString& aName, document* aParent, const QString& aID){
    m_parent = aParent;
    insert("name", aName);
    QString id;
    if (aID == "")
        id = rea::generateUUID();
    else
        id = aID;
    insert("id", id);
    aParent->m_coms.insert(id, this);
}

static rea::regPip<QQmlApplicationEngine*> init_gui_main([](rea::stream<QQmlApplicationEngine*>* aInput) {
    aInput->data()->load(QUrl(QStringLiteral("qrc:/GUI/main.qml")));
    static document doc;
    aInput->out();
}, rea::Json("name", "loadMain"));
