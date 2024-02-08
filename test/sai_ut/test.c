#include <stdio.h>
#include "sai.h"

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

int main()
{
    sai_status_t              status;
    sai_switch_api_t          *switch_api;
    sai_lag_api_t             *lag_api;
    sai_object_id_t           vr_oid;
    sai_object_id_t           lag_oid[2];
    sai_object_id_t           lag_member_oid[4];
    sai_attribute_t           attrs[2];
    sai_attribute_t           lag_member1_attrs[2];
    sai_attribute_t           lag_member2_attrs[2];
    sai_attribute_t           lag_member3_attrs[2];
    sai_attribute_t           lag_member4_attrs[2];
    sai_switch_notification_t notifications;
    sai_object_id_t           port_list[64];

    if ((status = sai_api_initialize(0, &test_services)) != SAI_STATUS_SUCCESS) {
        printf("sai_api_initialize() failed, status=%d\n", status);
        goto finish;
    }

    if ((status = sai_api_query(SAI_API_SWITCH, (void**)&switch_api)) != SAI_STATUS_SUCCESS) {
        printf("Failed to query SAI_API_SWITCH, status=%d\n", status);
        goto finish;
    }

    if ((status = switch_api->initialize_switch(0, "HW_ID", 0, &notifications)) != SAI_STATUS_SUCCESS) {
        printf("initialize_switch() failed, status=%d\n", status);
        goto cleanup;
    }

    if ((status = sai_api_query(SAI_API_LAG, (void**)&lag_api)) != SAI_STATUS_SUCCESS) {
        printf("Failed to query SAI_API_LAG, status=%d\n", status);
        goto cleanup;
    }

    attrs[0].id = SAI_SWITCH_ATTR_PORT_LIST;
    attrs[0].value.objlist.list = port_list;
    attrs[0].value.objlist.count = 64;
    status = switch_api->get_switch_attribute(1, attrs);
    for (int32_t ii = 0; ii < attrs[0].value.objlist.count; ii++) {
        printf("Port #%d OID: 0x%lX\n", ii, attrs[0].value.objlist.list[ii]);
    }

    printf("---------------------Start LAG Unit test--------------------\n");

    // Create LAG #1 and LAG_MEMBER #1,#2
    if ((status = lag_api->create_lag(&lag_oid[0], 0, NULL)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG #1, status=%d\n", status);
        goto cleanup;
    }

    lag_member1_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member1_attrs[0].value.u8 = 1;

    lag_member1_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    lag_member1_attrs[1].value.u8 = 1;

    if ((status = lag_api->create_lag_member(&lag_member_oid[0], 2, lag_member1_attrs)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG_MEMBER #1, status=%d\n", status);
        goto cleanup;
    }

    lag_member2_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member2_attrs[0].value.u8 = 1;

    lag_member2_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    lag_member2_attrs[1].value.u8 = 2;

    if ((status = lag_api->create_lag_member(&lag_member_oid[1], 0, lag_member2_attrs)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG_MEMBER #2, status=%d\n", status);
        goto cleanup;
    }

    // Create LAG #2 and LAG_MEMBER #3,#4

    if ((status = lag_api->create_lag(&lag_oid[1], 0, NULL)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG #2, status=%d\n", status);
        goto cleanup;
    }

    lag_member3_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member3_attrs[0].value.u8 = 2;

    lag_member3_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    lag_member3_attrs[1].value.u8 = 3;

    if ((status = lag_api->create_lag_member(&lag_member_oid[2], 0, lag_member3_attrs)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG_MEMBER #3, status=%d\n", status);
        goto cleanup;
    }

    lag_member4_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lag_member4_attrs[0].value.u8 = 2;

    lag_member4_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
    lag_member4_attrs[1].value.u8 = 4;

    if ((status = lag_api->create_lag_member(&lag_member_oid[3], 0, lag_member4_attrs)) != SAI_STATUS_SUCCESS) {
        printf("Failed to create LAG_MEMBER #4, status=%d\n", status);
        goto cleanup;
    }

    // Get LAG #1 port list
    sai_attribute_t* lag1_port_list = NULL;
    lag_api->get_lag_attribute(lag_oid[0], 1, lag1_port_list);

    // Get LAG #2 port list
    sai_attribute_t* lag2_port_list = NULL;
    lag_api->get_lag_attribute(lag_oid[1], 1, lag2_port_list);

    // Get LAG_MEMBER #1 LAG_ID
    sai_attribute_t* lag_member1_lag_id = NULL;
    lag_api->get_lag_member_attribute(lag_member_oid[0], 1, lag_member1_lag_id);

    // Get LAG_MEMBER #3 PORT_ID
    sai_attribute_t* lag_member3_port_id = NULL;
    lag_api->get_lag_member_attribute(lag_member_oid[2], 1, lag_member3_port_id);

    // Remove LAG_MEMBER #2
    lag_api->remove_lag_member(lag_member_oid[1]);

    // Get LAG #1 port list
    lag_api->get_lag_attribute(lag_oid[0], 1, lag1_port_list);

    // Remove LAG_MEMBER #3
    lag_api->remove_lag_member(lag_member_oid[2]);

    // Get LAG #2 port list
    lag_api->get_lag_attribute(lag_oid[0], 1, lag2_port_list);

    // Remove LAG_MEMBER #1
    lag_api->remove_lag_member(lag_member_oid[0]);

    // Remove LAG_MEMBER #4
    lag_api->remove_lag_member(lag_member_oid[3]);

    // Remove LAG #2
    lag_api->remove_lag(lag_oid[1]);

    // Remove LAG #1
    lag_api->remove_lag(lag_oid[0]);

    cleanup:
        switch_api->shutdown_switch(0);
        if ((status = sai_api_uninitialize()) != SAI_STATUS_SUCCESS)
            printf("sai_api_uninitialize() failed, status=%d\n", status);

    finish:
        return status;
}
