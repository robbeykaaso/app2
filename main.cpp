#include "main.h"
#include "storage0.h"
#include "command.h"

OBJTYPE(project)
OBJTYPE(folder)
OBJTYPE(page)
OBJTYPE(image)
OBJTYPE(text)
OBJTYPE(shape)
OBJTYPE(start)
OBJTYPE(assign)
OBJTYPE(judge)
OBJTYPE(redirect)
OBJTYPE(show)
OBJTYPE(function)
OBJTYPE(cloud_data)
OBJTYPE(focus)

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
        auto rg = cld.value("range").toArray();
        auto x = rg[0].toDouble(), y = rg[1].toDouble(), w = rg[2].toDouble(), h = rg[3].toDouble();
        ret.insert(cld.value("id").toString(), rea::Json(m_shape_template.value(cld.value("type").toString()).toObject(),
                                                         "points", rea::JArray(QJsonArray(), rea::JArray(x, y, x + w, y, x + w, y + h, x, y + h, x, y))));
    }
    return ret;
}

QJsonObject document::prepareRoutineView(const QJsonObject& aRoutineConfig){
    QJsonObject ret;
    auto clds = aRoutineConfig.value("children").toArray();
    for (auto i : clds){
        auto cld = i.toObject();
        auto shp = m_shape_template.value(cld.value("type").toString()).toObject();
        for (auto j : shp.keys()){
            if (j != "type" && cld.contains(j))
                shp.insert(j, cld.value(j));
        }
        ret.insert(cld.value("id").toString(), shp);
    }
    return ret;
}

QJsonValue document::modifyValue(const QJsonValue& aTarget, const QStringList& aKeys,
                          const int aIndex, const QJsonValue aValue) {
    if (aTarget.isObject()) {
        auto tar = aTarget.toObject();
        if (aIndex == aKeys.size() - 1)
            tar.insert(aKeys[aIndex], aValue);
        else
            tar.insert(aKeys[aIndex], modifyValue(tar.value(aKeys[aIndex]),
                                                     aKeys, aIndex + 1, aValue));
        return tar;
    } else if (aTarget.isArray()) {
        auto tar = aTarget.toArray();
        auto idx = aKeys[aIndex].toInt();
        while (idx >= tar.size()) tar.push_back(QJsonValue());
        if (aIndex == aKeys.size() - 1)
            tar[idx] = aValue;
        else
            tar[idx] = modifyValue(tar[idx], aKeys, aIndex + 1, aValue);
        return tar;
    } else
        assert(0);
}

void document::comManagement(){
    m_root_com = std::make_shared<projectObject>("element_root", this);
    m_sel_com = m_root_com.get();

    //select com
    rea::pipeline::find("_treeViewSelected")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            if (m_sel_com && m_sel_com->getType() == "page")
                aInput->out<QJsonArray>(rea::JArray(rea::Json("type", "select")), "updateQSGCtrl_elementend");
            m_sel_com = m_coms.value(aInput->data());
            if (!m_sel_com)
                m_sel_com = m_root_com.get();
            if (m_sel_com->getType() == "page"){
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
                            auto tp = obj.value("com_type").toString();
                            if (tp == "image")
                                m_sel_com->addChild(std::make_shared<imageObject>("image", this, mdy.value("tar").toString()))
                                        ->initialize(m_param_template.value(tp).toObject());
                            else if (tp == "shape")
                                m_sel_com->addChild(std::make_shared<shapeObject>("shape", this, mdy.value("tar").toString()))
                                        ->initialize(m_param_template.value(tp).toObject());
                            else if (tp == "text")
                                m_sel_com->addChild(std::make_shared<textObject>("text", this, mdy.value("tar").toString()))
                                        ->initialize(m_param_template.value(tp).toObject());
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
                                if (key == "points"){
                                    auto pts = mdy.value("val").toArray()[0].toArray();
                                    auto lt = QPointF(std::round(pts[0].toDouble()), std::round(pts[1].toDouble())),
                                         rb = QPointF(std::round(pts[4].toDouble()), std::round(pts[5].toDouble()));
                                    com->insert("range", rea::JArray(lt.x(), lt.y(), rb.x() - lt.x(), rb.y() - lt.y()));

                                    auto prm = QJsonObject(*com);
                                    prm.remove("id");
                                    prm.remove("type");
                                    if (m_sel_com && m_last_sel_board == "elementend")
                                        aInput->out<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
                                }
                            }
                        }
                    }
                }
                if (ch)
                    aInput->out<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
            }
        });

    //modify com parameter
    rea::pipeline::find("comtreeViewGUIModified")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto kys = dt.value("key").toString().split(";");
            if (m_sel_obj){
                QJsonArray old_pts;
                if (kys[1] == "range"){
                    auto rg = m_sel_obj->value("range").toArray();
                    auto x = rg[0].toDouble(), y = rg[1].toDouble(), w = rg[2].toDouble(), h = rg[3].toDouble();
                    old_pts = rea::JArray(x, y, x + w, y, x + w, y + h, x, y + h, x, y);
                }
                auto tmp = modifyValue(*m_sel_obj, kys, 1, dt.value("val")).toObject();
                m_sel_obj->insert(kys[1], tmp.value(kys[1]));
                if (kys[1] == "range"){
                    auto shp = tmp.value("id").toString();
                    auto rg = tmp.value("range").toArray();
                    auto x = rg[0].toDouble(), y = rg[1].toDouble(), w = rg[2].toDouble(), h = rg[3].toDouble();
                    auto new_pts = rea::JArray(x, y, x + w, y, x + w, y + h, x, y + h, x, y);
                    auto redo = [this, shp, new_pts](){
                        rea::pipeline::run("updateQSGAttr_" + m_last_sel_board, rea::Json("obj", shp,
                                                                                 "key", rea::JArray("points"),
                                                                                 "val", rea::JArray(QJsonArray(), new_pts)));
                    };
                    auto undo = [this, shp, old_pts](){
                        rea::pipeline::run("updateQSGAttr_" + m_last_sel_board, rea::Json("obj", shp,
                                                                                 "key", rea::JArray("points"),
                                                                                 "val", old_pts));
                    };
                    redo();
                    aInput->out<rea::ICommand>(rea::ICommand(redo, undo), "addCommand");
                }else if (kys[1] == "name"){
                    auto shp = tmp.value("id").toString();
                    aInput->out<QJsonObject>(rea::Json("obj", shp,
                                                       "key", rea::JArray("caption"),
                                                       "val", dt.value("val")), "updateQSGAttr_" + m_last_sel_board);
                }
            }
        });

    auto sel = m_root_com->getID();

    auto test = std::make_shared<pageObject>("page0", this);
    m_root_com->addChild(test);
    auto fld = std::make_shared<folderObject>("folder0", this);
    fld->addChild(std::make_shared<pageObject>("page1", this));
    m_root_com->addChild(fld);
    m_sel_com = test.get();
    sel = test->getID();

    rea::pipeline::run<QJsonObject>("_updateTreeView", rea::Json("data", m_root_com->generateDocument(),
                                                                 "select", sel));
}

void document::frontManagement(){
    m_root_front.push_back(std::make_shared<projectObject>("event1", this));
    m_sel_front = 0;

    //select event
    rea::pipeline::add<double>([this](rea::stream<double>* aInput){
        auto dt = int(aInput->data());
        if (m_sel_front != dt){
            m_sel_front = dt;
            aInput->out<QJsonObject>(rea::Json(m_page_template, "objects", prepareRoutineView(m_root_front[m_sel_front]->generateDocument())), "updateQSGModel_frontend");
        }
    }, rea::Json("name", "frontEventSelected"));

    //new event
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
                return;
            }
            m_root_front.push_back(std::make_shared<projectObject>(nm, this));
            aInput->out<QJsonObject>(prepareEventList(m_root_front, m_sel_front), "_updateFrontEventList");
        }, rea::Json("tag", "newFrontEvent"));

    //new com
    rea::pipeline::find("QSGAttrUpdated_frontend")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (m_sel_front < 0)
                return;
            auto cur = m_root_front[m_sel_front];
            for (auto i : dt){
                auto mdy = i.toObject();
                if (mdy.value("cmd").toBool()){
                    if (mdy.value("type") == "add"){
                        auto obj = mdy.value("val").toObject();
                        auto tp = obj.value("com_type").toString();
                        if (tp == "start")
                            cur->addChild(std::make_shared<startObject>("start", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "assign")
                            cur->addChild(std::make_shared<shapeObject>("assign", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "judge")
                            cur->addChild(std::make_shared<textObject>("judge", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "redirect")
                            cur->addChild(std::make_shared<textObject>("redirect", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "show")
                            cur->addChild(std::make_shared<textObject>("show", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "function")
                            cur->addChild(std::make_shared<textObject>("function", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "focus")
                            cur->addChild(std::make_shared<textObject>("focus", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                    }
                    else if (mdy.value("type") == "del"){
                        cur->removeChild(mdy.value("tar").toString());
                    }
                    else{
                        auto com = m_coms.value(mdy.value("obj").toString());
                        if (com){
                            auto key = mdy.value("key").toArray()[0].toString();
                            if (key == "points" || key == "center" || key == "radius"){
                                com->insert(key, mdy.value("val"));
                            }
                        }
                    }
                }
        }

        });

    rea::pipeline::run<QJsonObject>("_updateFrontEventList", prepareEventList(m_root_front, m_sel_front));
}

void document::backManagement(){
    m_root_back.push_back(std::make_shared<projectObject>("event2", this));
    m_sel_back = 0;

    //select event
    rea::pipeline::add<double>([this](rea::stream<double>* aInput){
        auto dt = int(aInput->data());
        if (m_sel_back != dt){
            m_sel_back = dt;
            aInput->out<QJsonObject>(rea::Json(m_page_template, "objects", prepareRoutineView(m_root_back[m_sel_back]->generateDocument())), "updateQSGModel_backend");
        }
    }, rea::Json("name", "backEventSelected"));

    //new event
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->out<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
                return;
            }
            m_root_back.push_back(std::make_shared<projectObject>(nm, this));
            aInput->out<QJsonObject>(prepareEventList(m_root_back, m_sel_back), "_updateBackEventList");
        }, rea::Json("tag", "newBackEvent"));

    //new com
    rea::pipeline::find("QSGAttrUpdated_backend")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (m_sel_back < 0)
                return;
            auto cur = m_root_back[m_sel_back];
            for (auto i : dt){
                auto mdy = i.toObject();
                if (mdy.value("cmd").toBool()){
                    if (mdy.value("type") == "add"){
                        auto obj = mdy.value("val").toObject();
                        auto tp = obj.value("com_type").toString();
                        if (tp == "start")
                            cur->addChild(std::make_shared<startObject>("start", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "assign")
                            cur->addChild(std::make_shared<shapeObject>("assign", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "judge")
                            cur->addChild(std::make_shared<textObject>("judge", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "redirect")
                            cur->addChild(std::make_shared<textObject>("redirect", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "show")
                            cur->addChild(std::make_shared<textObject>("show", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "function")
                            cur->addChild(std::make_shared<textObject>("function", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "cloud_data")
                            cur->addChild(std::make_shared<textObject>("cloud_data", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                    }
                    else if (mdy.value("type") == "del"){
                        cur->removeChild(mdy.value("tar").toString());
                    }
                    else{
                        auto com = m_coms.value(mdy.value("obj").toString());
                        if (com){
                            auto key = mdy.value("key").toArray()[0].toString();
                            if (key == "points" || key == "center" || key == "radius"){
                                com->insert(key, mdy.value("val"));
                            }
                        }
                    }
                }
        }

        });

    rea::pipeline::run<QJsonObject>("_updateBackEventList", prepareEventList(m_root_back, m_sel_back));
}

void document::initializeTemplate(){
    m_page_template = rea::Json("width", 400,
                                "height", 800,
                                "text", rea::Json("visible", true,
                                                  "size", rea::JArray(40, 20),
                                                  "location", "middle"));
    m_param_template = rea::Json("start", rea::Json("next", rea::Json("default", "")),
                                 "assign", rea::Json("variable", "",
                                                     "value", "",
                                                     "next", rea::Json("default", "")),
                                 "judge", rea::Json("vartiable", "",
                                                    "next", rea::Json("yes", "",
                                                                      "no", "")),
                                 "redirect", rea::Json("page", "",
                                                       "next", rea::Json("default", "")),
                                 "show", rea::Json("data", "",
                                                   "next", rea::Json("default", "")),
                                 "function", rea::Json("event", "",
                                                       "next", rea::Json("default", "")),
                                 "cloud_data", rea::Json("data", "",
                                                         "next", rea::Json("default", "")),
                                 "focus", rea::Json("data", "",
                                                    "next", rea::Json("default", "")),
                                 "image", rea::Json("name", "image0",
                                                    "range", QJsonArray(),
                                                    "comment", "",
                                                    "source", ""),
                                 "text", rea::Json("name", "text0",
                                                   "range", QJsonArray(),
                                                   "comment", "",
                                                   "content", "",
                                                   "size", 16,
                                                   "color", "green",
                                                   "bold", ""),
                                 "shape", rea::Json("name", "text0",
                                                    "range", QJsonArray(),
                                                    "comment", "",
                                                    "direction_color", "green",
                                                    "direction_border_type", "line",
                                                    "direction_border_color", "red",
                                                    "direction_radius", 30));
    m_shape_template = rea::Json(
                "start", rea::Json("type", "ellipse",
                                   "com_type", "start",
                                   "center", rea::JArray(0, 0),
                                   "radius", rea::JArray(50, 25),
                                   "color", "green",
                                   "face", "50",
                                   "caption", "start"),
                "assign", rea::Json("type", "poly",
                                   "com_type", "assign",
                                   "points", rea::JArray(QJsonArray(),
                                                         rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                   "color", "gray",
                                   "face", "50",
                                   "caption", "assign"),
                "judge", rea::Json("type", "poly",
                                   "com_type", "judge",
                                   "points", rea::JArray(QJsonArray(),
                                                         rea::JArray(0, 25, 50, 50, 100, 25, 50, 0, 0, 25)),
                                   "color", "blue",
                                   "face", "50",
                                   "caption", "judge"),
                "redirect", rea::Json("type", "poly",
                                      "com_type", "redirect",
                                      "points", rea::JArray(QJsonArray(),
                                                            rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                      "color", "red",
                                      "face", "50",
                                      "caption", "redirect"),
                "show", rea::Json("type", "poly",
                                  "com_type", "show",
                                  "points", rea::JArray(QJsonArray(),
                                                        rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                  "color", "orange",
                                  "face", "50",
                                  "caption", "show"),
                "focus", rea::Json("type", "ellipse",
                                   "com_type", "focus",
                                   "center", rea::JArray(0, 0),
                                   "radius", rea::JArray(15, 15),
                                   "color", "orange",
                                   "face", "50",
                                   "caption", "focus"
                                   ),
                "function", rea::Json("type", "poly",
                                      "com_type", "function",
                                      "points", rea::JArray(QJsonArray(),
                                                            rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                      "color", "orange",
                                      "face", "50",
                                      "caption", "function"),
                "cloud_data", rea::Json("type", "poly",
                                        "com_type", "cloud_data",
                                        "points", rea::JArray(QJsonArray(),
                                                              rea::JArray(0, 0, 100, 0, 100, 50, 0, 50, 0, 0)),
                                        "color", "green",
                                        "face", "50",
                                        "caption", "cloud_data"),
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
}

void document::regCreateShape(const QString& aType){
    auto tp = aType.toLower();
    rea::pipeline::add<QString>([this, tp](rea::stream<QString>* aInput){
        aInput->out<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                         m_shape_template.value(aInput->data()).toObject()
                                                         )), tp + "_pasteShapes");
    }, rea::Json("name", "create" + aType + "Com"));
}

QJsonObject document::prepareEventList(const std::vector<std::shared_ptr<comObject>>& aList, int aSelect){
    QJsonArray lst;
    for (auto i : aList)
        lst.push_back(i->getName());
    return rea::Json("data", lst,
                     "select", aSelect);
}

std::function<void(rea::stream<QJsonObject>*)> document::getShowParam(const QString& aBoardName){
    return [this, aBoardName](rea::stream<QJsonObject>* aInput){
        //auto tmp = m_sel_obj;
        //auto tmp2 = m_last_sel_board;
        //auto tmp3 = aBoardName;

        auto dt = aInput->data();
        auto shps = dt.value("shapes").toObject();
        QJsonObject prm;
        if (shps.size() > 0){
            if (m_sel_obj && m_last_sel_board != "" && m_last_sel_board != aBoardName)
                aInput->out<double>(0, m_last_sel_board + "_clearSelects");

            m_sel_obj = m_coms.value(shps.keys()[0]);
            prm = QJsonObject(*m_sel_obj);
            prm.remove("id");
            prm.remove("type");

            prm.remove("center");
            prm.remove("radius");
            prm.remove("points");
            m_last_sel_board = aBoardName;
        }else if (m_last_sel_board == aBoardName)
            m_sel_obj = nullptr;
        aInput->out<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
    };
}

document::document(){
    static rea::fsStorage0 stg;

    //save document
    rea::pipeline::find("_selectFile")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            auto pth = dt[0].toString();
            if (!pth.endsWith(".json"))
                pth += ".json";
            QJsonArray front, back;
            for (auto i : m_root_front)
                front.push_back(i->generateDocument());
            for (auto i : m_root_back)
                back.push_back(i->generateDocument());
            aInput->out<rea::stgJson>(rea::stgJson(rea::Json("element", m_root_com->generateDocument(),
                                                             "frontend", front,
                                                             "backend", back), pth), "writeJson");
        }, rea::Json("tag", "saveComModel"));

    initializeTemplate();
    comManagement();
    frontManagement();
    backManagement();
    regCreateShape("FrontEnd");
    regCreateShape("BackEnd");
    regCreateShape("ElementEnd");

    rea::pipeline::add<QJsonObject>(getShowParam("frontend"), rea::Json("name", "updateQSGSelects_frontend"));
    rea::pipeline::add<QJsonObject>(getShowParam("backend"), rea::Json("name", "updateQSGSelects_backend"));
    rea::pipeline::add<QJsonObject>(getShowParam("elementend"), rea::Json("name", "updateQSGSelects_elementend"));
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
