#ifndef VIEWPROPERTYEDITOR_H
#define VIEWPROPERTYEDITOR_H

#include <qttreepropertybrowser.h>
#include <qtvariantproperty.h>
#include <renderable.h>

class QSpinBox;

class MountainLabPropertyManager : public QtVariantPropertyManager {
    Q_OBJECT
public:
    MountainLabPropertyManager(QObject* parent = 0);

    bool isPropertyTypeSupported(int propertyType) const override;
    int valueType(int propertyType) const override;
    QStringList attributes(int propertyType) const override;
    int attributeType(int propertyType, const QString &attribute) const override;
    QVariant value(const QtProperty *property) const override;
    QVariant attributeValue(const QtProperty *property, const QString &attribute) const override;
    void setAttribute(QtProperty *property, const QString &attribute, const QVariant &value) override;

protected:
    QString valueText(const QtProperty *property) const override;
private:
    typedef QMap<QtProperty *, QString> PropertyStringMap;
    PropertyStringMap m_stringAttrs;

};

class MountainLabEditorFactory : public QtVariantEditorFactory {
    Q_OBJECT
public:
    MountainLabEditorFactory(QObject* parent = 0);

protected:
    QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent) override;
    void connectPropertyManager(QtVariantPropertyManager *manager) override;
    void disconnectPropertyManager(QtVariantPropertyManager *manager) override;
private slots:
    void editorDestroyed(QObject* editor);
    void attributeChanged(QtProperty *property, const QString &attribute, const QVariant &value);
private:
    QMap<QtProperty*, QList<QSpinBox*> > m_intPropertyToEditors;
    QMap<QSpinBox*, QtProperty* > m_intEditorToProperty;

};

class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtVariantProperty;
class QtProperty;
class ViewPropertyEditor : public QWidget
{
    Q_OBJECT
    Q_ENUMS(Mode)
public:
    ViewPropertyEditor(QWidget *parent = 0);
    enum Mode {
        Basic, Advanced
    };
    QtVariantPropertyManager *propertyManager() const;

    /* experimental API */
    void addProperty(const QString &propName, const QVariant &value, bool basic = true);
    void addRenderOptions(RenderOptionSet *set);
    void addRenderOptionsWithExtra(RenderOptionSet* main, RenderOptionSet *extra);

    QVariantMap values() const;
    void setValue(const QString &propName, const QVariant &value);
    QVariant value(const QString &propName) const;

    /* end of experimental API */
public slots:
    void apply();
signals:
    void propertiesChanged();
protected:
    void showEvent(QShowEvent *event);
    void relayoutProperties();
    void addRenderOptions(RenderOptionSet *set, QtProperty *parent);
private:
    QtVariantPropertyManager *m_propertyManager;
    QtVariantEditorFactory *m_editorFactory;
    QtTreePropertyBrowser *m_browser;
    QList<QtVariantProperty*> m_allProperties;
    QMap<QtProperty*, RenderOptionBase*> m_options;
    QMap<QtProperty*, RenderOptionSet*> m_sets;

};

#endif // VIEWPROPERTYEDITOR_H
