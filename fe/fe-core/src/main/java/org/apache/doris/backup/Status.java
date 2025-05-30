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

package org.apache.doris.backup;

public class Status {
    public enum ErrCode {
        OK,
        NOT_FOUND,
        BAD_FILE,
        CREATE_REMOTE_PATH_FAILED,
        IS_DIR,
        IS_FILE,
        TIMEOUT,
        BAD_CONNECTION,
        COMMON_ERROR,
        OLAP_VERSION_ALREADY_MERGED
    }

    private ErrCode errCode;
    private String errMsg;

    public static final Status OK = new Status(ErrCode.OK, "");

    public Status(ErrCode errCode, String errMsg) {
        this.errCode = errCode;
        this.errMsg = errMsg;
    }

    public ErrCode getErrCode() {
        return errCode;
    }

    public String getErrMsg() {
        return errMsg;
    }

    public boolean ok() {
        return errCode == ErrCode.OK;
    }

    @Override
    public boolean equals(Object other) {
        if (other instanceof Status s) {
            return errCode == s.getErrCode();
        } else {
            return this == other;
        }
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("[").append(errCode.name());
        if (!ok()) {
            sb.append(", msg: ").append(errMsg);
        }
        sb.append("]");
        return sb.toString();
    }
}
