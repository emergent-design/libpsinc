#pragma once
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <psinc/Camera.h>


class SettingsModel : public QAbstractTableModel
{
	Q_OBJECT

	public:

		SettingsModel(QObject *parent, psinc::Camera *camera) : QAbstractTableModel(parent), camera(camera) {}
		int rowCount(const QModelIndex &parent = QModelIndex()) const ;
		int columnCount(const QModelIndex &parent = QModelIndex()) const;
		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;

		void updateIndex();

	private:

		psinc::Camera *camera;
		std::vector<std::string> index;

	friend class FeatureDelegate;
};


class FeatureDelegate : public QItemDelegate
{
	Q_OBJECT

	public:

		FeatureDelegate(QObject *parent, SettingsModel *model) : QItemDelegate(parent), model(model) {}

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	private:

		SettingsModel *model;
};
