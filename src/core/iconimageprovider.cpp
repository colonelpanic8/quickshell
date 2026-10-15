#include "iconimageprovider.hpp"
#include <algorithm>

#include <qcolor.h>
#include <qicon.h>
#include <qlogging.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qstring.h>
#include <qurl.h>
#include <qurlquery.h>

namespace {

struct IconRequest {
	QString iconName;
	QString fallbackName;
	QString themePath;
};

IconRequest parseIconRequest(const QString& id) {
	auto request = IconRequest();
	auto splitIdx = id.indexOf('?');

	if (splitIdx == -1) {
		request.iconName = id;
		return request;
	}

	request.iconName = id.sliced(0, splitIdx);

	auto query = QUrlQuery(id.sliced(splitIdx + 1));
	request.themePath = query.queryItemValue("path", QUrl::FullyDecoded);
	request.fallbackName = query.queryItemValue("fallback", QUrl::FullyDecoded);
	return request;
}

} // namespace

QPixmap
IconImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) {
	auto request = parseIconRequest(id);

	auto icon = QIcon::fromTheme(request.iconName);
	if (icon.isNull() && !request.fallbackName.isEmpty()) {
		icon = QIcon::fromTheme(request.fallbackName);
	}

	if (icon.isNull() && !request.themePath.isEmpty()) {
		auto fileName = request.iconName.sliced(request.iconName.lastIndexOf('/') + 1);
		icon = QPixmap(QString("/%1/%2").arg(request.themePath, fileName));
	}

	auto targetSize = requestedSize.isValid() ? requestedSize : QSize(100, 100);
	if (targetSize.width() == 0 || targetSize.height() == 0) targetSize = QSize(2, 2);
	auto pixmap = icon.pixmap(targetSize.width(), targetSize.height());

	if (pixmap.isNull()) {
		qWarning() << "Could not load icon" << id << "at size" << targetSize << "from request";
		pixmap = IconImageProvider::missingPixmap(targetSize);
	}

	if (size != nullptr) *size = pixmap.size();
	return pixmap;
}

QPixmap IconImageProvider::missingPixmap(const QSize& size) {
	auto width = size.width() % 2 == 0 ? size.width() : size.width() + 1;
	auto height = size.height() % 2 == 0 ? size.height() : size.height() + 1;
	width = std::max(width, 2);
	height = std::max(height, 2);

	auto pixmap = QPixmap(width, height);
	pixmap.fill(QColorConstants::Black);
	auto painter = QPainter(&pixmap);

	auto halfWidth = width / 2;
	auto halfHeight = height / 2;
	auto purple = QColor(0xd900d8);
	painter.fillRect(halfWidth, 0, halfWidth, halfHeight, purple);
	painter.fillRect(0, halfHeight, halfWidth, halfHeight, purple);
	return pixmap;
}

QString IconImageProvider::requestString(
    const QString& icon,
    const QString& path,
    const QString& fallback
) {
	auto req = "image://icon/" + icon;
	auto query = QUrlQuery();

	if (!path.isEmpty()) {
		query.addQueryItem("path", QString::fromLatin1(QUrl::toPercentEncoding(path)));
	}

	if (!fallback.isEmpty()) {
		query.addQueryItem("fallback", QString::fromLatin1(QUrl::toPercentEncoding(fallback)));
	}

	if (!query.isEmpty()) {
		req += '?' + query.toString(QUrl::FullyEncoded);
	}

	return req;
}
