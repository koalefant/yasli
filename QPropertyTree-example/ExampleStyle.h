#include <QSplitter>

class QPropertyTree;
class ExampleStyleWidget : public QSplitter
{
	Q_OBJECT
public:
	ExampleStyleWidget();

	QPropertyTree* contentTree;
	QPropertyTree* styleTree;

private slots:
	void onStyleChanged();
	void onContentChanged();

	void onResetStyle();
};
