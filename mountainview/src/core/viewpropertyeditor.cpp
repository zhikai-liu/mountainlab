#include "viewpropertyeditor.h"
#include <QVBoxLayout>
#include <qtvariantproperty.h>
#include "qtpropertybrowserutils_p.h"
#include <QSpinBox>
#include <QtDebug>

ViewPropertyEditor::ViewPropertyEditor(QWidget *parent)
    : QWidget(parent)
    , m_propertyManager(new MountainLabPropertyManager(this))
    , m_editorFactory(new MountainLabEditorFactory(this))
    , m_browser(new QtTreePropertyBrowser)
{
    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->addWidget(m_browser);
    m_browser->setPropertiesWithoutValueMarked(true);
    m_browser->setAlternatingRowColors(true);

    m_browser->setFactoryForManager(m_propertyManager, m_editorFactory);
    connect(m_propertyManager, SIGNAL(propertyChanged(QtProperty*)), SIGNAL(propertiesChanged()));
}

QtVariantPropertyManager *ViewPropertyEditor::propertyManager() const
{
    return m_propertyManager;
}

void ViewPropertyEditor::addProperty(const QString &propName, const QVariant &value, bool basic)
{
    QtVariantProperty *property = m_propertyManager->addProperty(value.type(), propName);
    property->setValue(value);
    m_allProperties.append(property);
}

void ViewPropertyEditor::addRenderOptions(RenderOptionSet *set)
{
    QList<RenderOptionBase*> opts = set->options();
    QList<RenderOptionBase*> extOpts = set->extension() ? set->extension()->options() : QList<RenderOptionBase*>();
    opts += extOpts;
    qSort(opts.begin(), opts.end(), [](RenderOptionBase* o1, RenderOptionBase* o2) {
        return o1->name() < o2->name();
    });
    foreach(RenderOptionBase *option, opts) {
        QtVariantProperty *prop = m_propertyManager->addProperty(option->defaultValue().type(), option->name());
        if (!prop) {
            qWarning() << "Property type" << option->defaultValue().type() << "not supported";
            continue;
        }
        prop->setValue(option->value());
        foreach(const QString &attr, option->attributeNames()) {
            prop->setAttribute(attr, option->attribute(attr));
        }
        m_options.insert(prop, option);
        m_allProperties.append(prop);
    }
    foreach(RenderOptionSet *subset, set->sets()) {
        QtVariantProperty *group = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), subset->name());
        m_sets.insert(group, subset);
        m_allProperties.append(group);
        addRenderOptions(subset, group);
    }
}

void ViewPropertyEditor::addRenderOptionsWithExtra(RenderOptionSet* main, RenderOptionSet *extra)
{
    // first add extra options
    QList<RenderOptionBase*> opts = extra->options();
    qSort(opts.begin(), opts.end(), [](RenderOptionBase* o1, RenderOptionBase* o2) {
        return o1->name() < o2->name();
    });
    foreach(RenderOptionBase *option, opts) {
        QtVariantProperty *prop = m_propertyManager->addProperty(option->defaultValue().type(), option->name());
        prop->setValue(option->value());
        foreach(const QString &attr, option->attributeNames()) {
            prop->setAttribute(attr, option->attribute(attr));
        }
        m_options.insert(prop, option);
        m_allProperties.append(prop);
    }
    foreach(RenderOptionSet *subset, extra->sets()) {
        QtVariantProperty *group = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), subset->name());
        m_sets.insert(group, subset);
        m_allProperties.append(group);
        addRenderOptions(subset, group);
    }
    if (!main) return;
    // now add main options
    QtVariantProperty *mainGroup = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), main->name());
    m_allProperties.append(mainGroup);
    opts = main->options();
    QList<RenderOptionBase*> extOpts = main->extension() ? main->extension()->options() : QList<RenderOptionBase*>();
    opts += extOpts;
    qSort(opts.begin(), opts.end(), [](RenderOptionBase* o1, RenderOptionBase* o2) {
        return o1->name() < o2->name();
    });
    foreach(RenderOptionBase *option, opts) {
        QtVariantProperty *prop = m_propertyManager->addProperty(option->defaultValue().type(), option->name());
        if (!prop) {
            qWarning() << "Property of type" << option->defaultValue().type() << "not supported";
            continue;
        }
        prop->setValue(option->value());
        foreach(const QString &attr, option->attributeNames()) {
            prop->setAttribute(attr, option->attribute(attr));
        }
        m_options.insert(prop, option);
        mainGroup->addSubProperty(prop);
    }
    foreach(RenderOptionSet *subset, main->sets()) {
        QtVariantProperty *group = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), subset->name());
        m_sets.insert(group, subset);
        mainGroup->addSubProperty(group);
        addRenderOptions(subset, group);
    }
}

void ViewPropertyEditor::addRenderOptions(RenderOptionSet *set, QtProperty *parent)
{
    QList<RenderOptionBase*> opts = set->options();
    qSort(opts.begin(), opts.end(), [](RenderOptionBase* o1, RenderOptionBase* o2) {
        return o1->name() < o2->name();
    });
    foreach(RenderOptionBase *option, opts) {
        QtVariantProperty *prop = m_propertyManager->addProperty(option->defaultValue().type(), option->name());
        prop->setValue(option->value());
        foreach(const QString &attr, option->attributeNames()) {
            prop->setAttribute(attr, option->attribute(attr));
        }
        m_options.insert(prop, option);
        parent->addSubProperty(prop);
    }
    foreach(RenderOptionSet *subset, set->sets()) {
        QtVariantProperty *group = m_propertyManager->addProperty(QtVariantPropertyManager::groupTypeId(), subset->name());
        m_sets.insert(group, subset);
        parent->addSubProperty(group);
        addRenderOptions(subset, group);
    }
}

QVariantMap ViewPropertyEditor::values() const
{
    QVariantMap result;
    foreach(QtVariantProperty *prop, m_allProperties) {
        QString propName = prop->propertyName();
        QVariant value = prop->value();
        result.insert(propName, value);
    }
    return result;
}

void ViewPropertyEditor::setValue(const QString &propName, const QVariant &value)
{
    foreach(QtVariantProperty *prop, m_allProperties) {
        if (prop->propertyName() == propName) {
            prop->setValue(value);
            return;
        }
    }
}

QVariant ViewPropertyEditor::value(const QString &propName) const
{
    foreach(QtVariantProperty *prop, m_allProperties) {
        if (prop->propertyName() == propName) {
            return prop->value();
        }
    }
    return QVariant();
}

void ViewPropertyEditor::apply()
{
    QMapIterator<QtProperty*,RenderOptionBase*> iter(m_options);
    while (iter.hasNext()) {
        iter.next();
        QtVariantProperty *prop = static_cast<QtVariantProperty*>(iter.key());
        RenderOptionBase *opt = iter.value();
        opt->setValue(prop->value());
    }
}

void ViewPropertyEditor::showEvent(QShowEvent *event)
{
    relayoutProperties();
}

void ViewPropertyEditor::relayoutProperties()
{
    m_browser->clear();
    foreach(QtVariantProperty *prop, m_allProperties)
        m_browser->addProperty(prop);
}



MountainLabEditorFactory::MountainLabEditorFactory(QObject *parent) : QtVariantEditorFactory(parent) {}

QWidget *MountainLabEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
{
    QWidget *editor = 0;
    const int type = manager->propertyType(property);
    switch (type) {
    case QVariant::Bool: {
        editor = QtVariantEditorFactory::createEditor(manager, property, parent);
        QtBoolEdit *boolEdit = qobject_cast<QtBoolEdit *>(editor);
        if (boolEdit)
            boolEdit->setTextVisible(false);
    }
        break;
    case QVariant::Int: {
        editor = QtVariantEditorFactory::createEditor(manager, property, parent);
        QSpinBox* sb = (QSpinBox*)editor;
        const QVariant svt = manager->attributeValue(property, "specialValueText");
        if (svt.type() == QVariant::String)
            sb->setSpecialValueText(svt.toString());
        m_intPropertyToEditors[property].append(sb);
        m_intEditorToProperty[sb] = property;
        connect(editor, &QObject::destroyed, this, &MountainLabEditorFactory::editorDestroyed);
    }
        break;
    default:
        editor = QtVariantEditorFactory::createEditor(manager, property, parent);
        break;
    }
    return editor;
}

void MountainLabEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, &QtVariantPropertyManager::attributeChanged,
            this, &MountainLabEditorFactory::attributeChanged);
    QtVariantEditorFactory::connectPropertyManager(manager);
}

void MountainLabEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, &QtVariantPropertyManager::attributeChanged,
            this, &MountainLabEditorFactory::attributeChanged);
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

template<class Editor>
bool removeEditor(QObject* object, QMap<QtProperty*, QList<Editor> >* propertyToEditors,
                  QMap<Editor, QtProperty *> *editorToProperty) {
    if (!propertyToEditors) return false;
    if (!editorToProperty) return false;
    QMapIterator<Editor, QtProperty *> it(*editorToProperty);
    while (it.hasNext()) {
        Editor editor = it.next().key();
        if (editor == object) {
            QtProperty *prop = it.value();
            (*propertyToEditors)[prop].removeAll(editor);
            if ((*propertyToEditors)[prop].count() == 0)
                propertyToEditors->remove(prop);
            editorToProperty->remove(editor);
            return true;
        }
    }
    return false;
}

void MountainLabEditorFactory::editorDestroyed(QObject *editor)
{
    if (removeEditor(editor, &m_intPropertyToEditors, &m_intEditorToProperty))
        return;
}

template<class EditorList, class Editor, class SetterParameter, class Value>
void applyToEditors(const EditorList& list, void (Editor::*setter)(SetterParameter), const Value& value) {
    if (list.isEmpty()) {
        return;
    }
    for (auto it = list.constBegin(), end = list.constEnd(); it != end; ++it) {
        Editor &editor = *(*it);
        (editor.*setter)(value);
    }
}

void MountainLabEditorFactory::attributeChanged(QtProperty *property, const QString &attribute, const QVariant &value)
{
    QtVariantPropertyManager *manager = propertyManager(property);
    const int type = manager->propertyType(property);
    if (type == QVariant::Int && attribute == "specialValueText") {
        const QString val = value.toString();
        applyToEditors(m_intPropertyToEditors.value(property), &QSpinBox::setSpecialValueText, val);
    }
}

MountainLabPropertyManager::MountainLabPropertyManager(QObject *parent) : QtVariantPropertyManager(parent) {}

bool MountainLabPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int MountainLabPropertyManager::valueType(int propertyType) const
{
    return QtVariantPropertyManager::valueType(propertyType);
}

QStringList MountainLabPropertyManager::attributes(int propertyType) const
{
    QStringList ret = QtVariantPropertyManager::attributes(propertyType);
    if (propertyType == QVariant::Int) {
        ret.append("specialValueText");
    }
    return ret;
}

int MountainLabPropertyManager::attributeType(int propertyType, const QString &attribute) const
{
    if (propertyType == QVariant::Int && attribute == "specialValueText") {
        return QVariant::String;
    }
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant MountainLabPropertyManager::value(const QtProperty *property) const
{
    return QtVariantPropertyManager::value(property);
}

QVariant MountainLabPropertyManager::attributeValue(const QtProperty *property, const QString &attribute) const
{
    QtProperty* prop = const_cast<QtProperty*>(property);
    if (attribute == "specialValueText") {
        const PropertyStringMap::const_iterator it = m_stringAttrs.constFind(prop);
        if (it != m_stringAttrs.constEnd())
            return it.value();
        return QString();
    }
    return QtVariantPropertyManager::attributeValue(property, attribute);
}

void MountainLabPropertyManager::setAttribute(QtProperty *property, const QString &attribute, const QVariant &value)
{
    if (attribute == "specialValueText") {
        const PropertyStringMap::iterator it = m_stringAttrs.find(property);
        if (it != m_stringAttrs.end()) {
            if (it.value() == value) return;
            it.value() = value.toString();
        } else {
            m_stringAttrs[property] = value.toString();
        }
        emit attributeChanged(variantProperty(property), attribute, value);
    }
    QtVariantPropertyManager::setAttribute(property, attribute, value);
}

QString MountainLabPropertyManager::valueText(const QtProperty *property) const
{
    const int vType = QtVariantPropertyManager::valueType(property);
    if (vType == QVariant::Bool) {
        return QString();
    }
    if (vType == QVariant::Int) {
        const QVariant minAttr = attributeValue(property, "minimum");
        const QVariant svtAttr = attributeValue(property, "specialValueText");
        const QVariant val = value(property);
        if (minAttr.type() == QVariant::Int && svtAttr.type() == QVariant::String) {
            if (val == minAttr) return svtAttr.toString();
        }
    }
    return QtVariantPropertyManager::valueText(property);
}
