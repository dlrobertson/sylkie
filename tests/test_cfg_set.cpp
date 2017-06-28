#include <gtest/gtest.h>

extern "C" {
#include <cfg.h>
}

static struct cfg_parser parsers[] = {
    {'v', "verbosity", CFG_INT, "control the amound of logging"},
    {'b', "bar", CFG_WORD, "random word"},
    {'f', "foo", CFG_BYTE, "what would this be without foo"},
    {'o', "bool", CFG_BOOL, "throw a random bool in the middle"},
    {'z', "baz", CFG_STRING, "lesser known foobar"},
    {'q', "qux", CFG_HW_ADDRESS, "much lesser known than foobar"},
    {'e', "example", CFG_IPV6_ADDRESS, "random example"},
    {'!', "never-called", CFG_BOOL, "this is never called"}
};

static size_t parsers_sz = sizeof(parsers) / sizeof(struct cfg_parser);

static const char* parser_names[] = {
    "verbosity",
    "qux",
    "foo",
    "example",
    "bool",
    "baz",
    "bar",
};
static size_t parser_names_sz = sizeof(parser_names) / sizeof(const char*);

static const char* argv[] = {
    "-v",
    "42",
    "--bool",
    "-z",
    "thisisastring",
    "--example=ff02::1",
    "--foo",
    "255",
    "-b",
    "512",
    "-q",
    "00:00:00:00:00:00"
};
static int argc = sizeof(argv) / sizeof(const char*);

TEST(cfg, order) {
    size_t i;
    struct cfg_set set = {
        .usage="Simple Test Usage",
        .summary="Test for usage printer"
    };
    cfg_set_init_cmdline(&set, parsers, parsers_sz, argc, argv);
    for (i = 0; i < parsers_sz && i < parser_names_sz; ++i) {
        EXPECT_EQ(parser_names[i], set.options.map[i]->name);
    }
    cfg_set_free(&set);
}

TEST(cfg, get) {
    size_t i = 0;
    int res = 0;
    bool value = false, previous_value = false;
    struct cfg_set config_set = {
        .usage="executable [OPTIONS]",
        .summary="Test for usage printer",
    };
    const struct cfg_set_item* item;
    cfg_set_init_cmdline(&config_set, parsers, parsers_sz, argc, argv);
    for (i = 0; i < parsers_sz && i < parser_names_sz; ++i) {
        item = cfg_set_find(&config_set, parser_names[i]);
        ASSERT_TRUE(item);
        ASSERT_EQ(item->name, parser_names[i]);
    }
    item = cfg_set_find(&config_set, "never-called");
    ASSERT_FALSE(item);
    previous_value = value;
    res = cfg_set_find_type(&config_set, "never-called", CFG_BOOL, &value);
    ASSERT_EQ(res, -1);
    ASSERT_EQ(previous_value, value);
    cfg_set_free(&config_set);
}
