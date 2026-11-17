/* TEST_ONLY card fixtures. No physical-card UID is fabricated by firmware. */
#include "provision_data.h"
#include "access_control.h"
#include "rfid_service.h"
#include "fake_flash.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static access_control_t control;
static config_store_t store;
static app_command_t command;
static bool pending;
static uint32_t now;

static app_result_t send(const app_command_t *value, void *context) {
    (void)context;
    assert(!pending);
    command = *value;
    pending = true;
    return APP_OK;
}
static void reply(void) {
    app_event_t event;
    assert(pending);
    rfid_service_command(&store, &command, &event);
    pending = false;
    access_control_event(&control, &event, ++now);
}
static void keys(const char *text) {
    while (*text) {
        now += 10;
        access_control_key(&control, *text++, now);
    }
}
static void card(app_uid_t uid) {
    app_event_t event = {0};
    event.kind = APP_EVENT_CARD_PRESENT;
    event.uid = uid;
    event.occurred_ms = ++now;
    access_control_event(&control, &event, now);
}
static void reboot(void) {
    pending = false;
    access_control_init(&control, ++now, send, NULL);
    reply();
}
static void close_lock(void) {
    now += APP_UNLOCK_MS;
    access_control_tick(&control, now);
    assert(!control.view.unlocked);
}
static void admin(void) {
    keys("A654321#");
    reply();
    assert(control.admin && !control.view.unlocked);
}
int main(void) {
    _Static_assert(PROVISION_OCCUPIED_SLOTS == 8u, "Eight initial virtual cards");
    app_uid_t first = {.length=4, .bytes={0xf0,0x11,0x22,0x33}};
    app_uid_t second = {.length=7, .bytes={0xf0,0x44,0x55,0x66,0x77,0x88,0x99}};
    app_uid_t third = {.length=10, .bytes={0xf0,1,2,3,4,5,6,7,8,9}};
    test_flash_reset();
    assert(config_store_load(&store) == APP_NOT_READY);
    reboot();
    assert(control.view.state == APP_STATE_IDLE && control.view.used_count == 8);
    assert(control.view.occupied_mask == 0x3fc && !control.view.unlocked);
    assert(!store.config.slots[0].used && !store.config.slots[1].used);
    for (unsigned i=2; i<10; i++) {
        assert(store.config.slots[i].used);
        assert(app_uid_equal(&store.config.slots[i].uid, &provision_config.slots[i].uid));
        for (unsigned j=2; j<i; j++)
            assert(!app_uid_equal(&store.config.slots[i].uid, &store.config.slots[j].uid));
    }
    puts("PASS enrollment: erased Flash boots with eight distinct virtual UIDs in 03-10");
    card(first); reply();
    assert(control.view.result == APP_NOT_FOUND && !control.view.unlocked);
    admin();
    keys("1"); card(first); reply();
    assert(control.view.card_id == 1 && control.view.used_count == 9);
    keys("1"); card(second); reply();
    assert(control.view.card_id == 2 && control.view.used_count == 10);
    keys("1"); card(first); reply();
    assert(control.view.result == APP_DUPLICATE);
    keys("1"); card(third); reply();
    assert(control.view.result == APP_FULL);
    puts("PASS enrollment: Admin adds two observed card fixtures at 01/02; duplicate and full rejected");
    keys("203##"); reply(); /* Delete a virtual card. */
    assert(control.view.used_count == 9 && !store.config.slots[2].used);
    keys("*"); reboot();
    assert(control.view.used_count == 9 && !store.config.slots[2].used);
    assert(app_uid_equal(&store.config.slots[3].uid, &provision_config.slots[3].uid));
    admin();
    keys("202##"); reply(); /* Delete a physical-card fixture by number. */
    assert(control.view.used_count == 8 && !store.config.slots[1].used);
    keys("1"); card(third); reply();
    assert(control.view.card_id == 2 && control.view.used_count == 9);
    keys("1"); card(second); reply();
    assert(control.view.card_id == 3 && control.view.used_count == 10);
    keys("31123456#111111#111111#"); reply();
    assert(control.view.result == APP_OK);
    keys("*"); reboot();
    assert(control.view.used_count == 10);
    card(first); reply(); assert(control.view.unlocked && control.view.card_id == 1); close_lock();
    card(third); reply(); assert(control.view.unlocked && control.view.card_id == 2); close_lock();
    card(second); reply(); assert(control.view.unlocked && control.view.card_id == 3); close_lock();
    keys("123456#"); reply(); assert(!control.view.unlocked);
    keys("111111#"); reply(); assert(control.view.unlocked); close_lock();
    puts("PASS enrollment: delete real/virtual slots, lowest-free reuse, stable IDs and PIN survive reboot");
    unsigned char before[sizeof test_flash];
    memcpy(before, test_flash, sizeof before);
    assert(config_store_provision(&store, &provision_config) == APP_DENIED);
    assert(!memcmp(before, test_flash, sizeof before));
    test_flash_reset();
    test_flash[CONFIG_BANK_STRIDE + 1000] = 0; /* Damage outside record prefix. */
    memcpy(before, test_flash, sizeof before);
    assert(config_store_load(&store) == APP_STORAGE_CORRUPT);
    reboot();
    assert(control.view.state == APP_STATE_ERROR);
    assert(control.view.result == APP_STORAGE_CORRUPT && !control.view.unlocked);
    assert(!memcmp(before, test_flash, sizeof before));
    assert(config_store_provision(&store, &provision_config) == APP_STORAGE_CORRUPT);
    assert(!memcmp(before, test_flash, sizeof before));
    puts("PASS enrollment: corrupt Flash distinguished from erased Flash; initialization never overwrites corruption");
    test_flash_reset();
    test_write_budget = 5;
    reboot();
    assert(control.view.state == APP_STATE_ERROR && control.view.result == APP_IO_ERROR);
    test_write_budget = -1;
    memcpy(before, test_flash, sizeof before);
    reboot();
    assert(control.view.result == APP_STORAGE_CORRUPT && !control.view.unlocked);
    assert(!memcmp(before, test_flash, sizeof before));
    puts("PASS enrollment: interrupted first initialization stays locked and is not silently reseeded");
    return 0;
}
