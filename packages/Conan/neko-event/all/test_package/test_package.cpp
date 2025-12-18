#include <neko/event/event.hpp>

using namespace neko::event;

int main() {
    EventLoop loop;
    int total = 0;

    auto handlerId = loop.subscribe<int>([&](const int &value) {
        total += value;
    });

    loop.publish<int>(1, neko::Priority::Normal, neko::SyncMode::Sync);
    loop.publish<int>(2, neko::Priority::High, neko::SyncMode::Sync);

    // Ensure unsubscribe works and no further events are handled.
    loop.unsubscribe<int>(handlerId);
    loop.publish<int>(4, neko::Priority::Normal, neko::SyncMode::Sync);

    return (total == 3) ? 0 : 1;
}
