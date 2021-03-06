// Copyright (c) YugaByte, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied.  See the License for the specific language governing permissions and limitations
// under the License.
//

package org.yb.pgsql;

import org.junit.Test;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashSet;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static org.yb.AssertionWrappers.assertEquals;
import static org.yb.AssertionWrappers.assertFalse;
import static org.yb.AssertionWrappers.assertTrue;

import org.junit.runner.RunWith;

import org.yb.YBTestRunner;

@RunWith(value=YBTestRunner.class)
public class TestPgUpdate extends BasePgSQLTest {
  private static final Logger LOG = LoggerFactory.getLogger(TestPgUpdate.class);

  @Test
  public void testBasicUpdate() throws SQLException {
    String tableName = "test_basic_update";
    Set<Row> allRows = setupSimpleTable(tableName);

    // UPDATE with condition on partition columns.
    String query = String.format("SELECT h FROM %s WHERE h = 2 AND vi = 1000", tableName);
    try (Statement statement = connection.createStatement()) {
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(0, rcount);
      }
    }

    try (Statement statement = connection.createStatement()) {
      String update_txt = String.format("UPDATE %s SET vi = 1000 WHERE h = 2", tableName);
      statement.execute(update_txt);

      // Not allowing update primary key columns.
      update_txt = String.format("UPDATE %s SET r = 1000 WHERE h = 2", tableName);
      runInvalidQuery(statement, update_txt);
      update_txt = String.format("UPDATE %s SET h = h + 1 WHERE vi = 2", tableName);
      runInvalidQuery(statement, update_txt);
    }

    try (Statement statement = connection.createStatement()) {
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(10, rcount);
      }
    }

    // UPDATE with condition on regular columns.
    query = String.format("SELECT h FROM %s WHERE vi = 2000", tableName);
    try (Statement statement = connection.createStatement()) {
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(0, rcount);
      }
    }

    try (Statement statement = connection.createStatement()) {
      String update_txt = String.format("UPDATE %s SET vi = 2*vi WHERE vi = 1000", tableName);
      statement.execute(update_txt);
    }

    try (Statement statement = connection.createStatement()) {
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(10, rcount);
      }
    }
  }

  @Test
  public void testUpdateWithSingleColumnKey() throws SQLException {
    Set<Row> allRows = new HashSet<>();
    String tableName = "test_update_single_column_key";
    try (Statement statement = connection.createStatement()) {
      createSimpleTableWithSingleColumnKey(tableName);
      String insertTemplate = "INSERT INTO %s(h, r, vi, vs) VALUES (%d, %f, %d, '%s')";

      for (int h = 0; h < 10; h++) {
        int r = h + 100;
        statement.execute(String.format(insertTemplate, tableName,
                                        h, r + 0.5, h * 10 + r, "v" + h + r));
        allRows.add(new Row((long) h,
                            r + 0.5,
                            h * 10 + r,
                            "v" + h + r));
      }
    }

    try (Statement statement = connection.createStatement()) {
      String query = String.format("SELECT h FROM %s WHERE h > 5 AND vi = 1000", tableName);
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(0, rcount);
      }
    }

    try (Statement statement = connection.createStatement()) {
      String update_stmt = String.format("UPDATE %s SET vi = 1000 WHERE h > 5", tableName);
      statement.execute(update_stmt);

      // Not allowing update primary key columns.
      update_stmt = String.format("UPDATE %s SET h = h + 100 WHERE vi = 2", tableName);
      runInvalidQuery(statement, update_stmt);
    }

    try (Statement statement = connection.createStatement()) {
      String query = String.format("SELECT h FROM %s WHERE h > 5 AND vi = 1000", tableName);
      try (ResultSet rs = statement.executeQuery(query)) {
        int rcount = 0;
        while (rs.next()) rcount++;
        assertEquals(4, rcount);
      }
    }
  }
}
