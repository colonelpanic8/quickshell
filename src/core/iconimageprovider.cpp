#include "iconimageprovider.hpp"
#include <algorithm>

#include <qcolor.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qicon.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qset.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringbuilder.h>
#include <qstringlist.h>
#include <qtypes.h>
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

const auto ICON_SUFFIXES = QStringList {"png", "svg", "svgz", "xpm"};

// Theme directories are at most theme/size/context deep. The cap bounds the cost
// of a theme path pointing somewhere large like /usr/share/icons.
constexpr qsizetype MAX_THEME_DIR_DEPTH = 3;
constexpr qsizetype MAX_THEME_DIRS = 2048;

// Names to try for an icon, which may be given as a file path, file url, or with an extension.
QStringList iconNameCandidates(const QString& rawName) {
	auto name = rawName.trimmed();
	if (name.startsWith("file://")) name = QUrl(name).toLocalFile();
	if (name.isEmpty()) return {};

	auto candidates = QStringList();

	auto addWithoutSuffix = [&](const QString& name) {
		candidates.append(name);

		auto suffix = QFileInfo(name).suffix();
		if (ICON_SUFFIXES.contains(suffix, Qt::CaseInsensitive)) {
			candidates.append(name.chopped(suffix.length() + 1));
		}
	};

	addWithoutSuffix(name);

	auto fileName = QFileInfo(name).fileName();
	if (fileName != name) addWithoutSuffix(fileName);

	candidates.removeDuplicates();
	return candidates;
}

// The theme path itself, and if it is inside of or contains icon themes, their directories.
QStringList themePathDirs(const QString& themePath) {
	auto root = QDir(themePath);

	auto dir = root;
	for (auto i = 0; i <= MAX_THEME_DIR_DEPTH; i++) {
		if (dir.exists("index.theme")) {
			root = dir;
			break;
		}

		if (!dir.cdUp()) break;
	}

	auto dirs = QStringList {QDir(themePath).absolutePath()};
	auto level = QStringList {root.absolutePath()};

	for (auto depth = 0; depth <= MAX_THEME_DIR_DEPTH && !level.isEmpty(); depth++) {
		auto next = QStringList();

		for (const auto& path: level) {
			dirs.append(path);
			if (dirs.size() >= MAX_THEME_DIRS) return dirs;

			if (depth == MAX_THEME_DIR_DEPTH) continue;
			for (const auto& entry: QDir(path).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
				next.append(path % '/' % entry);
			}
		}

		level = next;
	}

	dirs.removeDuplicates();
	return dirs;
}

// Loads an icon from files rather than the global icon theme, as modifying the theme
// search paths is not thread safe and image providers run off the gui thread.
QIcon iconFromFiles(const QStringList& names, const QString& themePaths) {
	auto icon = QIcon();
	auto added = QSet<QString>();

	auto addFile = [&](const QString& file) {
		if (added.contains(file) || !QFileInfo(file).isFile()) return;
		added.insert(file);

		if (icon.isNull()) icon = QIcon(file);
		else icon.addFile(file);
	};

	for (const auto& name: names) {
		if (QFileInfo(name).isAbsolute()) addFile(name);
	}

	// dbusmenu sends a list of theme paths, which is joined with ':'.
	for (const auto& themePath: themePaths.split(':', Qt::SkipEmptyParts)) {
		for (const auto& dir: themePathDirs(themePath)) {
			for (const auto& name: names) {
				if (QFileInfo(name).isAbsolute()) continue;

				auto base = dir % '/' % name;
				addFile(base);

				for (const auto& suffix: ICON_SUFFIXES) {
					addFile(base % '.' % suffix);
				}
			}
		}
	}

	return icon;
}

QPixmap themedPixmap(const QStringList& names, const QSize& size) {
	for (const auto& name: names) {
		auto pixmap = QIcon::fromTheme(name).pixmap(size.width(), size.height());
		if (!pixmap.isNull()) return pixmap;
	}

	return QPixmap();
}

} // namespace

QPixmap
IconImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) {
	auto request = parseIconRequest(id);

	auto targetSize = requestedSize.isValid() ? requestedSize : QSize(100, 100);
	if (targetSize.width() == 0 || targetSize.height() == 0) targetSize = QSize(2, 2);
	auto pixmap = IconImageProvider::requestPixmapForIcon(
	    request.iconName,
	    request.themePath,
	    request.fallbackName,
	    targetSize
	);

	if (pixmap.isNull()) {
		qWarning() << "Could not load icon" << id << "at size" << targetSize << "from request";
		pixmap = IconImageProvider::missingPixmap(targetSize);
	}

	if (size != nullptr) *size = pixmap.size();
	return pixmap;
}

QPixmap IconImageProvider::requestPixmapForIcon(
    const QString& icon,
    const QString& path,
    const QString& fallback,
    const QSize& size
) {
	auto names = iconNameCandidates(icon);

	auto pixmap = themedPixmap(names, size);
	if (pixmap.isNull()) pixmap = themedPixmap(iconNameCandidates(fallback), size);
	if (pixmap.isNull()) pixmap = iconFromFiles(names, path).pixmap(size.width(), size.height());

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
