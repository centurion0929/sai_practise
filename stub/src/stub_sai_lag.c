/*
*  Copyright (C) 2014. Mellanox Technologies, Ltd. ALL RIGHTS RESERVED.
*
*    Licensed under the Apache License, Version 2.0 (the "License"); you may
*    not use this file except in compliance with the License. You may obtain
*    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
*    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
*    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
*    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
*    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
*    See the Apache Version 2.0 License for specific language governing
*    permissions and limitations under the License.
*
*/

#undef __MODULE__
#define __MODULE__ SAI_LAG

#include "sai.h"
#include "stub_sai.h"
#include "assert.h"

#define MAX_NUMBER_OF_LAG_MEMBERS 16
#define MAX_NUMBER_OF_LAGS 5
#define MAX_PORT 32

typedef struct _lag_member_db_entry_t {
    bool            is_used;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
} lag_member_db_entry_t;

typedef struct _lag_db_entry_t {
    bool            is_used;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db_entry_t;

struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db;

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
    "List of ports in LAG", SAI_ATTR_VAL_TYPE_OBJLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
    "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
    "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
    "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
    "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

sai_status_t get_lag_attribute(
    _In_ const sai_object_key_t *key,
    _In_ sai_attribute_value_t *value,
    _In_ uint32_t 				attr_index,
    _Inout_ vendor_cache_t		*cache,
    void 						*arg);

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t *key,
                                    _Inout_ sai_attribute_value_t *value,
                                    _In_ uint32_t 				attr_index,
                                    _Inout_ vendor_cache_t 		*cache,
                                    void 							*arg);

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
    { false, false, false, true },
    { false, false, false, true },
    get_lag_attribute, (void*)SAI_LAG_ATTR_PORT_LIST,
    NULL, NULL }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
    { true, false, false, true },
    { true, false, false, true },
    get_lag_member_attribute, (void*)SAI_LAG_MEMBER_ATTR_LAG_ID,
    NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
    { true, false, false, true },
    { true, false, false, true },
    get_lag_member_attribute, (void*)SAI_LAG_MEMBER_ATTR_PORT_ID,
    NULL, NULL }
};

int stub_can_remove_lag(const uint32_t lag_db_id)
{
    for (uint32_t ii = 0; ii < MAX_NUMBER_OF_LAG_MEMBERS; ++ii) {
        if (lag_db.lags[lag_db_id].members_ids[ii] != 0) {
            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status = 0;
    char list_str[MAX_LIST_VALUE_STR_LEN] = {};

    if (NULL == lag_id) {
        printf("NULL LAG id param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS != (status = check_attribs_metadata(attr_count, attr_list, lag_attribs,
    lag_vendor_attribs, SAI_OPERATION_CREATE))) {
        printf("Failed to attributes check\n");
        return status;
    }

    uint32_t ii = 0;

    for (; ii <= MAX_NUMBER_OF_LAGS; ++ii) {
        if (!lag_db.lags[ii].is_used)
            break;
    }

    if (ii == MAX_NUMBER_OF_LAGS) {
        printf("Cannot create LAG: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    uint32_t lag_db_id = ii;
    lag_db.lags[lag_db_id].is_used = true;

    sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, lag_db_id, lag_id);

    if (status == SAI_STATUS_SUCCESS)
        printf("CREATE LAG: 0x%1lX (%s)\n", *lag_id, list_str);
    else
        printf("Cannot create a LAG object\n");

    return status;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    printf("REMOVE LAG: 0x%1lX\n", lag_id);

    sai_status_t status = 0;
    uint32_t     lag_db_id = 0;

    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_id);

    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB ID\n");
        return status;
    }

    if (stub_can_remove_lag(lag_db_id) != SAI_STATUS_SUCCESS) {
        printf("Can't remove LAG #%u, members are in use\n", lag_db_id);
        return SAI_STATUS_FAILURE;
    }

    lag_db.lags[lag_db_id].is_used = false;
    memset(lag_db.lags[lag_db_id].members_ids, 0, sizeof(lag_db.lags[lag_db_id].members_ids));

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG attribute for 0x%1lX\n", lag_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("GET LAG attribute for 0x%1lX\n", lag_id);

    sai_object_key_t key = { .object_id = lag_id };

    return sai_get_attributes(&key, NULL, lag_attribs, lag_vendor_attribs, attr_count, attr_list);
}


sai_status_t get_lag_port_list(_In_ uint32_t db_id, _Out_ sai_attribute_value_t* value)
{
    sai_status_t status = 0;
    uint32_t port_list_id = 0;
    sai_attribute_value_t port_oid_val = {};
    sai_object_id_t port_list[MAX_PORT] = {};

    for (uint32_t ii = 0; ii != MAX_NUMBER_OF_LAG_MEMBERS; ++ii)
    {
        const sai_object_key_t key_lag_member = { .object_id = lag_db.lags[db_id].members_ids[ii] };
        if (key_lag_member.object_id && (status = get_lag_member_attribute(&key_lag_member, &port_oid_val, 0, NULL,
            (void*)SAI_LAG_MEMBER_ATTR_PORT_ID)) == SAI_STATUS_SUCCESS) {
            port_list[port_list_id++] = port_oid_val.oid;
        }
    }

    memcpy(value->objlist.list, port_list, port_list_id * sizeof(sai_object_id_t));
    value->objlist.count = port_list_id;

    return status;
}

sai_status_t get_lag_attribute(
    _In_ const sai_object_key_t *key,
    _In_ sai_attribute_value_t *value,
    _In_ uint32_t 				attr_index,
    _Inout_ vendor_cache_t		*cache,
    void 						*arg
)
{
    sai_status_t status = 0;
    uint32_t   db_index = 0;

    if ((status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG, &db_index) != SAI_STATUS_SUCCESS))
    {
        printf("Cannot get LAG DB index\n");
        return status;
    }

    switch((int64_t)arg) {
        case SAI_LAG_ATTR_PORT_LIST:
            if ((status = get_lag_port_list(db_index, value)) != SAI_STATUS_SUCCESS) {
                printf("Failed to get LAG port list\n");
                return status;
            }
            break;
        default:
            printf("Got unexpected attribute ID\n");
            return SAI_STATUS_FAILURE;
    }

    return status;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status = 0;
    char list_str[MAX_LIST_VALUE_STR_LEN] = {};
    const sai_attribute_value_t *lag_id = NULL, *port_id = NULL;
    uint32_t lag_id_idx = 0, port_id_idx = 0, data = 0;

    if (NULL == lag_member_id) {
        printf("NULL LAG_MEMBER id param\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (SAI_STATUS_SUCCESS != (status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs,
        lag_member_vendor_attribs, SAI_OPERATION_CREATE))) {
        printf("Failed to attributes check\n");
        return status;
    }

    uint32_t ii = 0;

    for (; ii <= MAX_NUMBER_OF_LAG_MEMBERS; ++ii) {
        if (!lag_db.members[ii].is_used)
            break;
    }

    if (ii == MAX_NUMBER_OF_LAG_MEMBERS) {
        printf("Cannot create LAG_MEMBER: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }

    uint32_t lag_member_db_id = ii;
    lag_db.members[ii].is_used = true;

    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);

    if ((status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID,
        &lag_id, &lag_id_idx)) != SAI_STATUS_SUCCESS) {
        printf("LAG_ID attribute not found\n");
        return status;
    }

    lag_db.members[lag_member_db_id].lag_oid = lag_id->oid;

    if ((status = find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID,
        &port_id, &port_id_idx)) != SAI_STATUS_SUCCESS) {
        printf("PORT_ID attribute not found\n");
        return status;
    }

    lag_db.members[lag_member_db_id].port_oid = port_id->oid;

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, lag_member_db_id, lag_member_id);

    if (status == SAI_STATUS_SUCCESS) {
        printf("CREATE LAG_MEMBER: 0x%1lX (%s)\n", *lag_member_id, list_str);
    }
    else {
        printf("Cannot create a LAG_MEMBER object\n");
    }

    status = stub_object_to_type(lag_id->oid, SAI_OBJECT_TYPE_LAG, &data);

    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB id\n");
        return status;
    }

    uint32_t lag_db_id = data;
    lag_db.lags[lag_db_id].members_ids[lag_member_db_id] = *lag_member_id;

    return status;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    printf("REMOVE LAG_MEMBER: 0x%1lX\n", lag_member_id);

    sai_status_t status = 0;
    uint32_t     lag_member_db_id = 0, lag_db_id = 0;

    if ((status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER,
        &lag_member_db_id)) != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG_MEMBER DB id\n");
        return status;
    }

    if ((status = stub_object_to_type(lag_db.members[lag_member_db_id].lag_oid, SAI_OBJECT_TYPE_LAG,
        &lag_db_id)) != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB id\n");
        return status;
    }

    memset(&lag_db.members[lag_member_db_id], 0, sizeof(lag_db.members[lag_member_db_id]));
    memset(&lag_db.lags[lag_db_id].members_ids[lag_member_db_id], 0, sizeof(sai_object_id_t));

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("SET LAG_MEMBER attribute for 0x%1lX\n", lag_member_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("GET LAG_MEMBER attribute for 0x%1lX\n", lag_member_id);

    const sai_object_key_t key = { .object_id = lag_member_id };

    return sai_get_attributes(&key, NULL, lag_member_attribs, lag_member_vendor_attribs, attr_count, attr_list);
}

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t *key,
                                    _Inout_ sai_attribute_value_t *value,
                                    _In_ uint32_t 				attr_index,
                                    _Inout_ vendor_cache_t 		*cache,
                                    void 							*arg)
{
    sai_status_t status = 0;
    uint32_t     db_index = 0;

    assert((SAI_LAG_MEMBER_ATTR_LAG_ID == (int64_t)arg) || (SAI_LAG_MEMBER_ATTR_PORT_ID == (int64_t)arg));

    if ((status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &db_index)))
    {
        printf("Cannot get LAG DB index\n");
        return status;
    }

    switch((int64_t)arg) {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            value->oid = lag_db.members[db_index].lag_oid;
            break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
            value->oid = lag_db.members[db_index].port_oid;
            break;
        default:
            printf("Got unexpected attribute ID\n");
            return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};
