
CREATE KEYSPACE chat WITH replication = {'class': 'org.apache.cassandra.locator.SimpleStrategy', 'replication_factor': '3'} AND durable_writes = true AND tablets = {'enabled': false};

CREATE TABLE chat.messages (
    room_id uuid,
    created_at timeuuid,
    content text,
    user_id uuid,
    username text,
    PRIMARY KEY (room_id, created_at)
) WITH CLUSTERING ORDER BY (created_at DESC)
    AND bloom_filter_fp_chance = 0.01
    AND caching = {'keys': 'ALL', 'rows_per_partition': 'ALL'}
    AND comment = ''
    AND compaction = {'class': 'SizeTieredCompactionStrategy'}
    AND compression = {'sstable_compression': 'org.apache.cassandra.io.compress.LZ4Compressor'}
    AND crc_check_chance = 1
    AND default_time_to_live = 0
    AND gc_grace_seconds = 864000
    AND max_index_interval = 2048
    AND memtable_flush_period_in_ms = 0
    AND min_index_interval = 128
    AND speculative_retry = '99.0PERCENTILE'
    AND tombstone_gc = {'mode': 'timeout', 'propagation_delay_in_seconds': '3600'};


CREATE TABLE chat.rooms (
    room_id uuid,
    admin_ids set<uuid>,
    created_at timestamp,
    name text,
    owner_id uuid,
    user_ids set<uuid>,
    PRIMARY KEY (room_id)
) WITH bloom_filter_fp_chance = 0.01
    AND caching = {'keys': 'ALL', 'rows_per_partition': 'ALL'}
    AND comment = ''
    AND compaction = {'class': 'SizeTieredCompactionStrategy'}
    AND compression = {'sstable_compression': 'org.apache.cassandra.io.compress.LZ4Compressor'}
    AND crc_check_chance = 1
    AND default_time_to_live = 0
    AND gc_grace_seconds = 864000
    AND max_index_interval = 2048
    AND memtable_flush_period_in_ms = 0
    AND min_index_interval = 128
    AND speculative_retry = '99.0PERCENTILE'
    AND tombstone_gc = {'mode': 'timeout', 'propagation_delay_in_seconds': '3600'};


CREATE INDEX rooms_name_idx ON chat.rooms(name);


CREATE INDEX rooms_user_ids_idx ON chat.rooms(values(user_ids));


CREATE TABLE chat.users (
    user_id uuid,
    created_at timestamp,
    password text,
    room_ids set<uuid>,
    salt text,
    username text,
    PRIMARY KEY (user_id)
) WITH bloom_filter_fp_chance = 0.01
    AND caching = {'keys': 'ALL', 'rows_per_partition': 'ALL'}
    AND comment = ''
    AND compaction = {'class': 'SizeTieredCompactionStrategy'}
    AND compression = {'sstable_compression': 'org.apache.cassandra.io.compress.LZ4Compressor'}
    AND crc_check_chance = 1
    AND default_time_to_live = 0
    AND gc_grace_seconds = 864000
    AND max_index_interval = 2048
    AND memtable_flush_period_in_ms = 0
    AND min_index_interval = 128
    AND speculative_retry = '99.0PERCENTILE'
    AND tombstone_gc = {'mode': 'timeout', 'propagation_delay_in_seconds': '3600'};


CREATE INDEX users_username_idx ON chat.users(username);

