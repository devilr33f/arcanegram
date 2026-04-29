// ag_sync_initdata.h
#pragma once

#include <QtCore/QString>
#include <functional>
#include <memory>

namespace Main { class Session; }

namespace Arcanegram::Sync {

struct InitData {
    QString raw;     // urlencoded query string, ready for Authorization: tma <raw>
    qint64 authDate = 0;
};

class InitDataFetcher {
public:
    InitDataFetcher();
    ~InitDataFetcher();

    using Callback = std::function<void(InitData)>;
    using ErrorCallback = std::function<void(QString)>;

    void setSession(Main::Session *session);   // null = detached
    void request(Callback ok, ErrorCallback fail);
    void invalidate();

private:
    struct Private;
    std::unique_ptr<Private> _p;
};

} // namespace Arcanegram::Sync
