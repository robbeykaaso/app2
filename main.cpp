#include "main.h"
#include "storage0.h"
#include "command.h"
#include "qsgBoard.h"

class qsgPluginSelectApp2 : public rea::qsgPluginTransform{
private:
    bool m_no_affine;
    rea::pipe0* m_hide_handle = nullptr;

    QRectF calcSelectsBound(const QSet<QString>& aSelects, const QMap<QString, std::shared_ptr<rea::qsgObject>>& aShapes){
        QRectF bnd;
        int idx = 0;
        for (auto i : aSelects){
            auto cur = reinterpret_cast<rea::shapeObject*>(aShapes.value(i).get())->getBoundBox();
            if (idx++){
                bnd = QRectF(QPointF(std::min(bnd.left(), cur.left()), std::min(bnd.top(), cur.top())),
                             QPointF(std::max(bnd.right(), cur.right()), std::max(bnd.bottom(), cur.bottom())));
            }else
                bnd = cur;
        }
        return bnd;
    }
public:
    qsgPluginSelectApp2(std::shared_ptr<rea::qsgBoardPlugin> aOriginSelect, const QJsonObject& aConfig) : qsgPluginTransform(aConfig){
        m_origin_select = aOriginSelect;
        m_no_affine = aConfig.value("tag") == "noAffine";
    }
    ~qsgPluginSelectApp2() override{
        if (m_hide_handle){
            rea::pipeline::removeAspect(getParentName() + "_updateSelectedMask", rea::pipe0::AspectType::AspectAround, m_hide_handle->actName());
            rea::pipeline::remove(m_hide_handle->actName());
        }
    }
protected:
    void wheelEvent(QWheelEvent *event) override{
        m_origin_select->wheelEvent(event);
    }
    void mousePressEvent(QMouseEvent *event) override{
        m_origin_select->mousePressEvent(event);
    }
    void beforeDestroy() override{
        m_origin_select->beforeDestroy();
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        m_origin_select->mouseReleaseEvent(event);
    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
        m_origin_select->hoverMoveEvent(event);
    }
    QString getName(rea::qsgBoard* aParent = nullptr) override{
        rea::qsgBoardPlugin::getName(aParent);

        if (m_no_affine)
            m_hide_handle = rea::pipeline::add<QSet<QString>>([this](rea::stream<QSet<QString>>* aInput){
                auto sels = aInput->data();
                rea::pointList pts;
                if (sels.size() > 0){
                    auto bnd = calcSelectsBound(aInput->data(), getQSGModel()->getQSGObjects());
                    pts.push_back(bnd.topLeft());
                    pts.push_back(bnd.topRight());
                    pts.push_back(bnd.bottomRight());
                    pts.push_back(bnd.bottomLeft());
                }
                //aInput->var<QSet<QString>>("selects", sels)->outs<rea::pointList>(pts);
            }, rea::Json("around", getParentName() + "_updateSelectedMask"));

        return m_origin_select->getName(aParent);
    }
private:
    std::shared_ptr<rea::qsgBoardPlugin> m_origin_select;
};

static rea::regPip<QJsonObject, rea::pipePartial> plugin_select([](rea::stream<QJsonObject>* aInput){
    if (aInput->data().value("tag") == "noAffine")
        aInput->var<std::shared_ptr<rea::qsgBoardPlugin>>("result",
                                                          std::make_shared<qsgPluginSelectApp2>(aInput->varData<std::shared_ptr<rea::qsgBoardPlugin>>("result"),
                                                                                                aInput->data()))
        ->out();
}, rea::Json("after", "create_qsgboardplugin_select"));

class qsgPluginDrawLink : public rea::qsgPluginTransform{
private:
    rea::pipe0* m_set_link_type;
public:
    qsgPluginDrawLink(const QJsonObject& aConfig) : qsgPluginTransform(aConfig){
        m_set_link_type = rea::pipeline::find("_newObject")
            ->next(rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
                auto dt = aInput->data();
                m_link_type = dt.value("yes").toBool() ? "yes" : "no";
                aInput->outs<double>(0);
            })->nextBF<double>([this](rea::stream<double>* aInput){
                addLink(m_shape, getGeometry(), m_start)();
            }, "", rea::Json("name", "addDefaultLink")), "selectLinkType");
    }
    ~qsgPluginDrawLink() override{
        rea::pipeline::remove(m_set_link_type->actName());
        rea::pipeline::remove("addDefaultLink");
        if (m_parent)
            m_parent->setCursor(Qt::CursorShape::ArrowCursor);
    }
private:
    QJsonArray getGeometry(){
        return rea::JArray(QJsonArray(),
                           QJsonArray({m_st.x(), m_st.y(),
                                       m_ed.x(), m_ed.y()}));
    }
    void tryCancel(){
        if (m_shape != ""){
            removeShape(m_shape)();
            m_shape = "";
        }
    }
    std::function<void(void)> addLink(const QString& aShape, const QJsonArray& aPoints, const QString aStart, const QString aEnd = ""){
        auto nm = getParentName();
        auto id = getQSGModel()->value("id");
        auto tp = m_link_type;
        return [nm, aShape, aPoints, id, aStart, aEnd, tp](){
            auto val = rea::Json(
                "type", "poly",
                "tag", "link",
                "points", aPoints,
                "color", "grey",
                "arrow", rea::Json("visible", true));
            if (tp == "default")
                val.insert("text", rea::Json("visible", false));
            else
                val.insert("caption", tp);
            rea::pipeline::run("updateQSGAttr_" + nm,
                               rea::Json("key", rea::JArray("objects"),
                                         "type", "add",
                                         "tar", aShape,
                                         "val", val,
                                         "pole", QJsonArray({aStart, aEnd}),
                                         "id", id), "addLink");
        };
    }
protected:
    QString m_shape;
    QPointF m_st, m_ed;
    QString m_start;
    QString m_link_type;
    QString getName(rea::qsgBoard* aParent = nullptr) override {
        if (aParent)
            aParent->setCursor(QCursor(QPixmap("cursor.png")));
        return qsgPluginTransform::getName(aParent);
    }
    void beforeDestroy() override{
        tryCancel();
        qsgPluginTransform::beforeDestroy();
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        qsgPluginTransform::mouseReleaseEvent(event);
        if (event->button() == Qt::LeftButton){
            std::shared_ptr<rea::qsgObject> sel = nullptr;
            QString st;
            auto objs = getQSGModel()->getQSGObjects();
            for (auto i : objs.keys()){
                auto obj = objs.value(i);
                if (obj->value("tag") != "link" && obj->bePointSelected(m_wcspos.x(), m_wcspos.y())){
                    st = i;
                    sel = objs.value(i);
                    break;
                }
            }
            if (!sel)
                return;
            m_wcspos = std::dynamic_pointer_cast<rea::shapeObject>(sel)->getBoundBox().center();
            if (m_shape == ""){
                m_shape = newShapeID();
                m_st = m_wcspos;
                m_ed = m_st;
                m_start = st;
                m_link_type = "default";
                rea::pipeline::run<QString>("selectLinkType", m_start, "selectLinkType");
            }else{
                m_ed = m_wcspos;
                rea::pipeline::run("updateQSGAttr_" + getParentName(),
                                   rea::Json("obj", m_shape,
                                             "key", QJsonArray({"points"}),
                                             "val", getGeometry(),
                                             "force", true,
                                             "pole", rea::JArray(m_start, st),
                                             "id", getQSGModel()->value("id")),
                                   "drawLine");
                rea::pipeline::run<rea::ICommand>("addCommand",
                                                  rea::ICommand(addLink(m_shape, getGeometry(), m_start, st),
                                                                removeShape(m_shape)),
                                                  "manual");
                m_shape = "";
            }
        }
    }
    void hoverMoveEvent(QHoverEvent *event) override {
        qsgPluginTransform::hoverMoveEvent(event);
        if (m_shape != ""){
            m_ed = m_wcspos;
            rea::pipeline::run("updateQSGAttr_" + getParentName(),
                               rea::Json("obj", m_shape,
                                         "key", QJsonArray({"points"}),
                                         "val", getGeometry()),
                               "drawLink");
        }
    }
private:
    QImage m_img;
};

static rea::regPip<QQmlApplicationEngine*> init_drawline([](rea::stream<QQmlApplicationEngine*>* aInput){
        rea::pipeline::add<QJsonObject, rea::pipePartial>([](rea::stream<QJsonObject>* aInput){
            aInput->var<std::shared_ptr<rea::qsgBoardPlugin>>("result", std::make_shared<qsgPluginDrawLink>(aInput->data()))->out();
        }, rea::Json("name", "create_qsgboardplugin_drawlink"));
}, QJsonObject(), "regQML");


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

        auto nxt = cld.value("next").toObject();
        for (auto i : nxt.keys()){
            auto dflt = nxt.value(i).toString();
            if (m_coms.contains(dflt)){
                QJsonObject lnk = *m_coms.value(dflt);
                lnk.insert("type", "poly");
                ret.insert(dflt, lnk);
            }
        }
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

    //select page for data
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        QJsonArray pgs;
        QJsonArray nms;
        for (auto i : m_coms.keys())
            if (m_coms.value(i)->getType() == "page"){
                nms.push_back(m_coms.value(i)->getName());
                pgs.push_back(i);
            }
        aInput->setData(rea::Json("title", "pages", "content", rea::Json("value", rea::Json("model", nms, "value", pgs))))->out();
    }, rea::Json("name", "_selectPage"));

    //select com
    rea::pipeline::find("_treeViewSelected")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            if (m_sel_com && m_sel_com->getType() == "page")
                aInput->outs<QJsonArray>(rea::JArray(rea::Json("type", "select")), "updateQSGCtrl_elementend");
            m_sel_com = m_coms.value(aInput->data());
            if (!m_sel_com)
                m_sel_com = m_root_com.get();
            if (m_sel_com->getType() == "page"){
                aInput->outs<QJsonObject>(rea::Json(m_page_template, "objects", preparePageView(m_sel_com->generateDocument())), "updateQSGModel_elementend");
            }
        }, "manual");

    //new folder
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (!m_sel_com)
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "no parent com!"), "popMessage");
            else if (m_sel_com->getType() == "project" || m_sel_com->getType() == "folder")
                m_sel_com->addChild(std::make_shared<folderObject>(dt.value("name").toString(), m_sel_com->getParent()));
            else
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "create failed!"), "popMessage");
            aInput->outs<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
        }, "newFolder");

    //new page
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (!m_sel_com)
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "no parent com!"), "popMessage");
            else if (m_sel_com->getType() == "project" || m_sel_com->getType() == "folder")
                m_sel_com->addChild(std::make_shared<pageObject>(dt.value("name").toString(), m_sel_com->getParent()));
            else
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "create failed!"), "popMessage");
            aInput->outs<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
        }, "newPage");

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
                                        aInput->outs<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
                                }
                            }
                        }
                    }
                }
                if (ch)
                    aInput->outs<QJsonObject>(rea::Json("data", m_root_com->generateDocument()), "_updateTreeView");
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
                    aInput->outs<rea::ICommand>(rea::ICommand(redo, undo), "addCommand");
                }else if (kys[1] == "name"){
                    auto shp = tmp.value("id").toString();
                    aInput->outs<QJsonObject>(rea::Json("obj", shp,
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

    //select event for data
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        QJsonArray pgs;
        QJsonArray nms;
        for (auto i : m_root_front){
            nms.push_back(i->getName());
            pgs.push_back(i->getID());
        }
        aInput->setData(rea::Json("title", "events", "content", rea::Json("value", rea::Json("model", nms, "value", pgs))))->out();
    }, rea::Json("name", "_selectFrontEvent"));

    //update data value
    rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
        if (m_sel_front_data && m_sel_front_data->getType() == "data"){
            m_sel_front_data->insert("value", aInput->data());
        }
    }, rea::Json("name", "frontdata_updateSelectedDataValue"));

    //select data
    rea::pipeline::find("frontdata_treeViewSelected")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            m_sel_front_data = m_coms.value(aInput->data());
            aInput->outs<QString>(m_sel_front_data->value("value").toString(), "frontdata_updateSelectedDataValueGUI");
        }, "manual");

    //new data
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (m_sel_front < 0)
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "no parent event!"), "popMessage");
            else{
                auto cld = std::make_shared<dataObject>(dt.value("name").toString(), this);
                cld->insert("atype", "data");
                cld->insert("type", dt.value("type"));
                cld->insert("value", dt.value("value").toString());
                if (!m_sel_front_data)
                    m_sel_front_data = m_root_front[m_sel_front].get();
                if (m_sel_front_data == m_root_front[m_sel_front].get())
                    m_root_front[m_sel_front]->addChildData(cld);
                else
                    m_sel_front_data->addChild(cld);
                aInput->outs<QString>(m_sel_front_data->value("value").toString(), "frontdata_updateSelectedDataValueGUI");
            }
            aInput->outs<QJsonObject>(rea::Json("data", m_root_front[m_sel_front]->generateDataDocument()), "frontdata_updateTreeView");
        }, "new_frontdata", rea::Json("name", "new_frontdata"));

    //select event
    rea::pipeline::add<double>([this](rea::stream<double>* aInput){
        auto dt = int(aInput->data());
        if (m_sel_front != dt){
            m_sel_front = dt;
            aInput->outs<QJsonObject>(rea::Json(m_page_template, "objects", prepareRoutineView(m_root_front[m_sel_front]->generateDocument())), "updateQSGModel_frontend");
        }
        aInput->outs<QJsonObject>(rea::Json("data", m_root_front[m_sel_front]->generateDataDocument(),
                                           "select", m_root_front[m_sel_front]->getDataRoot()->getID()), "frontdata_updateTreeView");
        aInput->outs<QString>("", "frontdata_updateSelectedDataValueGUI");
    }, rea::Json("name", "frontEventSelected"));

    //new event
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
                return;
            }
            m_root_front.push_back(std::make_shared<projectObject>(nm, this));
            aInput->outs<QJsonObject>(prepareEventList(m_root_front, m_sel_front), "_updateFrontEventList");
        }, "newFrontEvent");

    //selectLinkType
    rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
        if  (m_coms.value(aInput->data())->value("type").toString() == "judge"){
            aInput->outs<QJsonObject>(rea::Json("title", "select link type", "content", rea::Json("yes", true)), "_newObject");
        }else{
            m_link_type = "default";
            aInput->outs<double>(0, "addDefaultLink");
        }
    }, rea::Json("name", "selectLinkType"))
        ->next("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            m_link_type = dt.value("yes").toBool() ? "yes" : "no";
        }, "selectLinkType");

    //new front com
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
                            cur->addChild(std::make_shared<assignObject>("assign", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "judge")
                            cur->addChild(std::make_shared<judgeObject>("judge", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "redirect")
                            cur->addChild(std::make_shared<redirectObject>("redirect", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "show")
                            cur->addChild(std::make_shared<showObject>("show", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "function")
                            cur->addChild(std::make_shared<functionObject>("function", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "focus")
                            cur->addChild(std::make_shared<focusObject>("focus", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                    }
                    else if (mdy.value("type") == "del"){
                        cur->removeChild(mdy.value("tar").toString());
                    }
                    else{
                        auto id = mdy.value("obj").toString();
                        auto com = m_coms.value(id);
                        if (com){
                            auto key = mdy.value("key").toArray()[0].toString();
                            if (key == "points" || key == "center" || key == "radius"){
                                com->insert(key, mdy.value("val"));
                            }
                        }
                    }
                }else{
                    auto poles = mdy.value("pole").toArray();
                    if (poles.size() > 0){
                        if (mdy.value("type") == "add"){
                            auto lnk = std::make_shared<linkObject>("", this, mdy.value("tar").toString());
                            m_links.push_back(lnk);
                            auto cfg = mdy.value("val").toObject();
                            cfg.remove("type");
                            lnk->initialize(rea::Json(cfg, "start", poles[0]));
                        }else{
                            auto id = mdy.value("obj").toString();
                            auto com = m_coms.value(id);
                            if (com){
                                auto key = mdy.value("key").toArray()[0].toString();
                                if (key == "points"){
                                    com->insert(key, mdy.value("val"));
                                }
                                auto st = poles[0].toString();
                                m_coms.value(st)->insert("next", rea::Json(m_coms.value(st)->value("next").toObject(), m_link_type, id));
                                com->insert("end", poles[1].toString());
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

    //select event for data
    rea::pipeline::add<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
        QJsonArray pgs;
        QJsonArray nms;
        for (auto i : m_root_back){
            nms.push_back(i->getName());
            pgs.push_back(i->getID());
        }
        aInput->setData(rea::Json("title", "cloud", "content", rea::Json("value", rea::Json("model", nms, "value", pgs))))->out();
    }, rea::Json("name", "_selectBackEvent"));

    //update data value
    rea::pipeline::add<QString>([this](rea::stream<QString>* aInput){
        if (m_sel_back_data && m_sel_back_data->getType() == "data"){
            m_sel_back_data->insert("value", aInput->data());
        }
    }, rea::Json("name", "backdata_updateSelectedDataValue"));

    //select data
    rea::pipeline::find("backdata_treeViewSelected")
        ->nextF<QString>([this](rea::stream<QString>* aInput){
            m_sel_back_data = m_coms.value(aInput->data());
            aInput->outs<QString>(m_sel_back_data->value("value").toString(), "backdata_updateSelectedDataValueGUI");
        }, "manual");

    //new data
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
            }else if (m_sel_back < 0)
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "no parent event!"), "popMessage");
            else{
                if (!m_sel_back_data)
                    m_sel_back_data = m_root_front[m_sel_back].get();
                if (m_sel_back_data == m_root_front[m_sel_back].get())
                    m_root_back[m_sel_back]->addChildData(std::make_shared<dataObject>(dt.value("name").toString(), this));
                else
                    m_sel_back_data->addChild(std::make_shared<dataObject>(dt.value("name").toString(), this));
                aInput->outs<QString>(m_sel_back_data->value("value").toString(), "backdata_updateSelectedDataValueGUI");
            }
            aInput->outs<QJsonObject>(rea::Json("data", m_root_back[m_sel_back]->generateDataDocument()), "backdata_updateTreeView");
        }, "new_backdata", rea::Json("name", "new_backdata"));

    //select event
    rea::pipeline::add<double>([this](rea::stream<double>* aInput){
        auto dt = int(aInput->data());
        if (m_sel_back != dt){
            m_sel_back = dt;
            aInput->outs<QJsonObject>(rea::Json(m_page_template, "objects", prepareRoutineView(m_root_back[m_sel_back]->generateDocument())), "updateQSGModel_backend");
        }
        aInput->outs<QJsonObject>(rea::Json("data", m_root_back[m_sel_back]->generateDataDocument(),
                                           "select", m_root_back[m_sel_back]->getDataRoot()->getID()), "backdata_updateTreeView");
        aInput->outs<QString>("", "backdata_updateSelectedDataValueGUI");
    }, rea::Json("name", "backEventSelected"));

    //new event
    rea::pipeline::find("_newObject")
        ->nextF<QJsonObject>([this](rea::stream<QJsonObject>* aInput){
            auto dt = aInput->data();
            if (dt.empty())
                return;
            auto nm = dt.value("name").toString();
            if (nm == ""){
                aInput->outs<QJsonObject>(rea::Json("title", "warning", "text", "invalid name!"), "popMessage");
                return;
            }
            m_root_back.push_back(std::make_shared<projectObject>(nm, this));
            aInput->outs<QJsonObject>(prepareEventList(m_root_back, m_sel_back), "_updateBackEventList");
        }, "newBackEvent");

    //new back com
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
                            cur->addChild(std::make_shared<assignObject>("assign", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "judge")
                            cur->addChild(std::make_shared<judgeObject>("judge", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "redirect")
                            cur->addChild(std::make_shared<redirectObject>("redirect", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "show")
                            cur->addChild(std::make_shared<showObject>("show", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "function")
                            cur->addChild(std::make_shared<functionObject>("function", this, mdy.value("tar").toString()))
                                    ->initialize(m_param_template.value(tp).toObject());
                        else if (tp == "cloud_data")
                            cur->addChild(std::make_shared<cloud_dataObject>("cloud_data", this, mdy.value("tar").toString()))
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
                                 "shape", rea::Json("name", "shape0",
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
    rea::pipeline::run<QJsonObject>("updateQSGModel_elementend", rea::Json(m_page_template,
                                                                           "objects", rea::Json("background",
                                                                                                rea::Json(
                                                                                                    "type", "image",
                                                                                                    "path", "phone.png",
                                                                                                    "range", rea::JArray(0, 0, 400, 800),
                                                                                                    "text", rea::Json("visible", false)))));
}

void document::regCreateShape(const QString& aType){
    auto tp = aType.toLower();
    rea::pipeline::add<QString>([this, tp](rea::stream<QString>* aInput){
        aInput->outs<QJsonObject>(rea::Json("shapes", rea::JArray(
                                                         m_shape_template.value(aInput->data()).toObject()
                                                         )), tp + "_pasteShapes");
    }, rea::Json("name", "create" + aType + "Com"));
}

QJsonObject document::prepareEventList(const std::vector<std::shared_ptr<projectObject>>& aList, int aSelect){
    QJsonArray lst;
    for (auto i : aList)
        lst.push_back(i->getName());
    return rea::Json("data", lst,
                     "select", aSelect);
}

void document::getShowParam(const QString& aBoardName){
    rea::pipeline::find(aBoardName + "_selectedChanged")
        ->nextF<QSet<QString>>([this, aBoardName](rea::stream<QSet<QString>>* aInput){
        //auto tmp = m_sel_obj;
        //auto tmp2 = m_last_sel_board;
        //auto tmp3 = aBoardName;

        auto shps = aInput->data();
        QJsonObject prm;
        if (shps.size() > 0){
            if (m_sel_obj && m_last_sel_board != "" && m_last_sel_board != aBoardName)
                aInput->outs<double>(0, m_last_sel_board + "_clearSelects");

            m_sel_obj = m_coms.value(*shps.begin());
            prm = QJsonObject(*m_sel_obj);
            prm.remove("id");
            prm.remove("type");

            prm.remove("center");
            prm.remove("radius");
            prm.remove("points");
            m_last_sel_board = aBoardName;
        }else if (m_last_sel_board == aBoardName)
            m_sel_obj = nullptr;
        aInput->outs<QJsonObject>(rea::Json("data", prm), "comloadTreeView");
    });
}

document::document(){
    static rea::fsStorage0 stg;

    //save document
    rea::pipeline::find("_selectFile")
        ->nextF<QJsonArray>([this](rea::stream<QJsonArray>* aInput){
            auto dt = aInput->data();
            if (dt.size() == 0)
                return;
            auto pth = dt[0].toString();
            if (!pth.endsWith(".json"))
                pth += ".json";
            QJsonArray front, back, alldata;
            for (auto i : m_root_front){
                front.push_back(i->generateDocument());
                alldata.push_back(i->generateDataDocument());
            }
            for (auto i : m_root_back){
                back.push_back(i->generateDocument());
                alldata.push_back(i->generateDataDocument());
            }
            aInput->outs<rea::stgJson>(rea::stgJson(rea::Json("element", m_root_com->generateDocument(),
                                                             "frontend", front,
                                                             "backend", back,
                                                             "alldata", alldata), pth), "writeJson");
        }, "saveComModel");

    initializeTemplate();
    comManagement();
    frontManagement();
    backManagement();
    regCreateShape("FrontEnd");
    regCreateShape("BackEnd");
    regCreateShape("ElementEnd");

    //getShowParam("frontend");
    //getShowParam("backend");
    getShowParam("elementend");
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
