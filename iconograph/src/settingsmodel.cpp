#include "settingsmodel.h"
#include <QSpinBox>


int SettingsModel::rowCount(const QModelIndex &/*parent*/) const
{
	return this->index.size();
}


int SettingsModel::columnCount(const QModelIndex &/*parent*/) const
{
	return 2;
}


QVariant SettingsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		switch(index.column())
		{
			case 0: return QString::fromStdString(this->index[index.row()]);
			case 1: return this->camera->features[this->index[index.row()]].Get();
		}

	}

	return {};
}



QVariant SettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
			case 0:	return "Name";
			case 1:	return "Value";
		}
	}


	return QVariant();
}


void SettingsModel::updateIndex()
{
	this->index.clear();
	this->index.reserve(this->camera->features.size());

	for (auto &f : this->camera->features)
	{
		this->index.emplace_back(f.first);
	}

	emit layoutChanged();
}


bool SettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role == Qt::EditRole)
	{
		this->camera->features[this->index[index.row()]].Set(value.toInt());
	}

	return true;
}


Qt::ItemFlags SettingsModel::flags(const QModelIndex &index) const
{
	bool editable = index.column() == 1 && !this->camera->features[this->index[index.row()]].ReadOnly();

	return editable ? Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled : Qt::ItemIsEnabled;
}



QWidget *FeatureDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
	auto *editor = new QSpinBox(parent);

	if (index.row() < (int)this->model->index.size())
	{
		auto &feature = this->model->camera->features[this->model->index[index.row()]];

		editor->setMinimum(feature.Minimum());
		editor->setMaximum(feature.Maximum());
	}

	return editor;
}


void FeatureDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	auto *spin = static_cast<QSpinBox *>(editor);

	spin->setValue(this->model->camera->features[this->model->index[index.row()]].Get());
}



void FeatureDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	auto *spin = static_cast<QSpinBox *>(editor);

	spin->interpretText();
	model->setData(index, spin->value(), Qt::EditRole);
}

void FeatureDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
	editor->setGeometry(option.rect);
}
