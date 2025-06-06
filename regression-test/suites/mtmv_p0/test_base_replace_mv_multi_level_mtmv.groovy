// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

import org.junit.Assert;

suite("test_base_replace_mv_multi_level_mtmv","mtmv") {
    String dbName = context.config.getDbNameByFile(context.file)
    String suiteName = "test_base_replace_mv_multi_level_mtmv"
    String tableName1 = "${suiteName}_table1"
    String tableName2 = "${suiteName}_table2"
    String mvName1 = "${suiteName}_mv1"
    String mvName11 = "${suiteName}_mv11"
    String mvName2 = "${suiteName}_mv2"
    String mvName3 = "${suiteName}_mv3"
    String mvName4 = "${suiteName}_mv4"
    String querySql = "SELECT t1.k1,t1.k2,t2.k4 from ${tableName1} t1 join ${tableName2} t2 on t1.k1=t2.k3;";
    sql """set enable_materialized_view_nest_rewrite = true;"""
    sql """drop table if exists `${tableName1}`"""
    sql """drop table if exists `${tableName2}`"""
    sql """drop materialized view if exists ${mvName1};"""
    sql """drop materialized view if exists ${mvName11};"""
    sql """drop materialized view if exists ${mvName2};"""
    sql """drop materialized view if exists ${mvName3};"""
    sql """drop materialized view if exists ${mvName4};"""

    sql """
        CREATE TABLE ${tableName1}
        (
            k1 INT,
            k2 varchar(32)
        )
        DISTRIBUTED BY HASH(k1) BUCKETS 2
        PROPERTIES (
            "replication_num" = "1"
        );
        """
    sql """
        CREATE TABLE ${tableName2}
        (
            k3 INT,
            k4 varchar(32)
        )
        DISTRIBUTED BY HASH(k3) BUCKETS 2
        PROPERTIES (
            "replication_num" = "1"
        );
        """
    sql """
            INSERT INTO ${tableName1} VALUES(1,"a");
        """
    sql """
        INSERT INTO ${tableName2} VALUES(1,"b");
    """
    sql """
        CREATE MATERIALIZED VIEW ${mvName1}
        BUILD DEFERRED REFRESH AUTO ON MANUAL
        DISTRIBUTED BY hash(k1) BUCKETS 2
        PROPERTIES (
        'replication_num' = '1'
        )
        AS
        SELECT * from ${tableName1};
        """
    sql """
            REFRESH MATERIALIZED VIEW ${mvName1} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName1)

    sql """
        CREATE MATERIALIZED VIEW ${mvName11}
        BUILD DEFERRED REFRESH AUTO ON MANUAL
        DISTRIBUTED BY hash(k1) BUCKETS 2
        PROPERTIES (
        'replication_num' = '1'
        )
        AS
        SELECT * from ${tableName1};
        """
    sql """
            REFRESH MATERIALIZED VIEW ${mvName11} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName11)

    sql """
        CREATE MATERIALIZED VIEW ${mvName2}
        BUILD DEFERRED REFRESH AUTO ON MANUAL
        DISTRIBUTED BY hash(k3) BUCKETS 2
        PROPERTIES (
        'replication_num' = '1'
        )
        AS
        SELECT * from ${tableName2};
        """
    sql """
            REFRESH MATERIALIZED VIEW ${mvName2} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName2)

    sql """
        CREATE MATERIALIZED VIEW ${mvName3}
        BUILD DEFERRED REFRESH AUTO ON MANUAL
        DISTRIBUTED BY hash(k1) BUCKETS 2
        PROPERTIES (
        'replication_num' = '1'
        )
        AS
        SELECT t1.k1,t1.k2,t2.k4 from ${tableName1} t1 join ${tableName2} t2 on t1.k1=t2.k3;
        """
    sql """
            REFRESH MATERIALIZED VIEW ${mvName3} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName3)

    sql """
        CREATE MATERIALIZED VIEW ${mvName4}
        BUILD DEFERRED REFRESH AUTO ON MANUAL
        DISTRIBUTED BY hash(k1) BUCKETS 2
        PROPERTIES (
        'replication_num' = '1'
        )
        AS
        SELECT t1.k1,t1.k2,t2.k4 from ${mvName1} t1 join ${mvName2} t2 on t1.k1=t2.k3;
        """
    sql """
            REFRESH MATERIALIZED VIEW ${mvName4} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName4)

    // replace mv1
    sql """
        ALTER MATERIALIZED VIEW ${mvName1} REPLACE WITH MATERIALIZED VIEW ${mvName11} PROPERTIES('swap' = 'true');;
        """
    order_qt_replace_true_mv_mv1 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName1}'"
    order_qt_replace_true_mv_mv11 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName11}'"
    order_qt_replace_true_mv_mv2 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName2}'"
    order_qt_replace_true_mv_mv3 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName3}'"
    order_qt_replace_true_mv_mv4 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName4}'"
    // replace table will rename default partition name, so will change to async
    mv_not_part_in(querySql, mvName1)
    mv_not_part_in(querySql, mvName11)
    mv_rewrite_success_without_check_chosen(querySql, mvName2)
    mv_rewrite_success_without_check_chosen(querySql, mvName3)
    mv_not_part_in(querySql, mvName4)

    // after refresh,should can rewrite
    sql """
            REFRESH MATERIALIZED VIEW ${mvName1} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName1)
    mv_rewrite_success_without_check_chosen(querySql, mvName1)

    // after refresh,should can rewrite
    sql """
            REFRESH MATERIALIZED VIEW ${mvName11} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName11)
    mv_rewrite_success_without_check_chosen(querySql, mvName11)

    // replace mv1
    sql """
        ALTER MATERIALIZED VIEW ${mvName1} REPLACE WITH MATERIALIZED VIEW ${mvName11} PROPERTIES('swap' = 'false');;
        """
    order_qt_replace_false_mv_mv1 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName1}'"
    order_qt_replace_false_mv_mv11 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName11}'"
    order_qt_replace_false_mv_mv2 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName2}'"
    order_qt_replace_false_mv_mv3 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName3}'"
    order_qt_replace_false_mv_mv4 "select Name,State,RefreshState  from mv_infos('database'='${dbName}') where Name='${mvName4}'"
    // replace table will rename default partition name, so will change to async
    mv_not_part_in(querySql, mvName1)
    mv_rewrite_success_without_check_chosen(querySql, mvName2)
    mv_rewrite_success_without_check_chosen(querySql, mvName3)
    mv_not_part_in(querySql, mvName4)

    // after refresh,should can rewrite
    sql """
            REFRESH MATERIALIZED VIEW ${mvName1} auto
        """
    waitingMTMVTaskFinishedByMvName(mvName1)
    mv_rewrite_success_without_check_chosen(querySql, mvName1)
}
