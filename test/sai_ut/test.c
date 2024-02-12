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

bool check_lag_port_list(sai_object_id_t* port_list, uint32_t count_actual, const sai_object_id_t* lag_expected_port_list,
	uint32_t count_expected);

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
		goto deinit;
	}

	if ((status = switch_api->initialize_switch(0, "HW_ID", 0, &notifications)) != SAI_STATUS_SUCCESS) {
		printf("initialize_switch() failed, status=%d\n", status);
		goto deinit;
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
	lag_member1_attrs[0].value.oid = lag_oid[0];

	lag_member1_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
	lag_member1_attrs[1].value.oid = port_list[0];

	if ((status = lag_api->create_lag_member(&lag_member_oid[0], 2, lag_member1_attrs)) != SAI_STATUS_SUCCESS) {
		printf("Failed to create LAG_MEMBER #1, status=%d\n", status);
		goto cleanup;
	}

	lag_member2_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
	lag_member2_attrs[0].value.oid = lag_oid[0];

	lag_member2_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
	lag_member2_attrs[1].value.oid = port_list[1];

	if ((status = lag_api->create_lag_member(&lag_member_oid[1], 2, lag_member2_attrs)) != SAI_STATUS_SUCCESS) {
		printf("Failed to create LAG_MEMBER #2, status=%d\n", status);
		goto cleanup;
	}

	// Create LAG #2 and LAG_MEMBER #3,#4
	if ((status = lag_api->create_lag(&lag_oid[1], 0, NULL)) != SAI_STATUS_SUCCESS) {
		printf("Failed to create LAG #2, status=%d\n", status);
		goto cleanup;
	}

	lag_member3_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
	lag_member3_attrs[0].value.oid = lag_oid[1];

	lag_member3_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
	lag_member3_attrs[1].value.oid = port_list[2];

	if ((status = lag_api->create_lag_member(&lag_member_oid[2], 2, lag_member3_attrs)) != SAI_STATUS_SUCCESS) {
		printf("Failed to create LAG_MEMBER #3, status=%d\n", status);
		goto cleanup;
	}

	lag_member4_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
	lag_member4_attrs[0].value.oid = lag_oid[1];

	lag_member4_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID,
	lag_member4_attrs[1].value.oid = port_list[3];

	if ((status = lag_api->create_lag_member(&lag_member_oid[3], 2, lag_member4_attrs)) != SAI_STATUS_SUCCESS) {
		printf("Failed to create LAG_MEMBER #4, status=%d\n", status);
		goto cleanup;
	}

	// Get LAG #1 port list
	sai_attribute_t lag1_port_list_attr[1] = {};
	sai_object_id_t lag1_port_list_oids[32] = {};
	const sai_object_id_t lag1_expected_results[] = { port_list[0], port_list[1] };

	lag1_port_list_attr[0].id = SAI_LAG_ATTR_PORT_LIST;
	lag1_port_list_attr[0].value.objlist.list = lag1_port_list_oids;
	lag1_port_list_attr[0].value.objlist.count = 32;

	if ((status = lag_api->get_lag_attribute(lag_oid[0], 1, lag1_port_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to get LAG #1 PORT MEMBER LIST\n");
		goto cleanup;
	}

	if (!check_lag_port_list(lag1_port_list_oids, lag1_port_list_attr[0].value.objlist.count, lag1_expected_results,
		sizeof(lag1_expected_results) / sizeof(lag1_expected_results[0]))) {
		printf("Test GET LAG #1 port list failed\n");
	}

	// Get LAG #2 port list
	sai_attribute_t lag2_port_list_attr[1] = {};
	sai_object_id_t lag2_port_list_oids[32] = {};
	const sai_object_id_t lag2_expected_results[] = { port_list[2], port_list[3] };

	lag2_port_list_attr[0].id = SAI_LAG_ATTR_PORT_LIST;
	lag2_port_list_attr[0].value.objlist.list = lag2_port_list_oids;
	lag2_port_list_attr[0].value.objlist.count = 32;

	if ((status = lag_api->get_lag_attribute(lag_oid[1], 1, lag2_port_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to get LAG #2 PORT MEMBER LIST\n");
		goto cleanup;
	}

	if (!check_lag_port_list(lag2_port_list_oids, lag2_port_list_attr[0].value.objlist.count, lag2_expected_results,
		sizeof(lag2_expected_results) / sizeof(lag2_expected_results[0]))) {
		printf("Test GET LAG#2 port list failed\n");
	}

	// Get LAG_MEMBER #1 LAG_ID
	sai_attribute_t lag_member1_list_attr[1] = { {.id = SAI_LAG_MEMBER_ATTR_LAG_ID} };
	sai_object_id_t expected_lag_id = lag_oid[0];

	if ((status = lag_api->get_lag_member_attribute(lag_member_oid[0], 1, lag_member1_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to get LAG_MEMBER #1 LAG ID\n");
		goto cleanup;
	}

	printf("LAG_MEMBER #1 LAG_ID 0x%lX\n", lag_member1_attrs[0].value.oid);

	if (lag_member1_list_attr[0].value.oid != expected_lag_id) {
		printf("Test check LAG_MEMBER #1 LAG_ID failed\n");
	}

	// Get LAG_MEMBER #3 PORT_ID
	sai_attribute_t lag_member3_list_attr[1] = { {.id = SAI_LAG_MEMBER_ATTR_PORT_ID} };
	sai_object_id_t expected_port_id = port_list[2];

	if ((status = lag_api->get_lag_member_attribute(lag_member_oid[2], 1, lag_member3_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to GET LAG_MEMBER #3 PORT_ID\n");
		goto cleanup;
	}

	printf("LAG_MEMBER #3 PORT_ID 0x%lX\n", lag_member3_list_attr[0].value.oid);

	if (lag_member3_list_attr[0].value.oid != expected_port_id) {
		printf("Test check LAG_MEMBER #3 PORT_ID failed\n");
	}

	// Remove LAG_MEMBER #2
	if ((status = lag_api->remove_lag_member(lag_member_oid[1])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #2\n");
		goto cleanup;
	}

	// Get LAG #1 port list
	if ((status = lag_api->get_lag_attribute(lag_oid[0], 1, lag1_port_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to get LAG #1 port list\n");
		goto cleanup;
	}

	const sai_object_id_t lag1_expected_results_test2[] = { port_list[0] };

	if (!check_lag_port_list(lag1_port_list_oids, lag1_port_list_attr[0].value.objlist.count, lag1_expected_results_test2,
		sizeof(lag1_expected_results_test2) / sizeof(lag1_expected_results_test2[0]))) {
		printf("Test #2 GET LAG #1 port list failed\n");
	}

	// Remove LAG_MEMBER #3
	if ((status = lag_api->remove_lag_member(lag_member_oid[2])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #3\n");
		goto cleanup;
	}

	const sai_object_id_t lag2_expected_results_test2[] = { port_list[3] };

	// Get LAG #2 port list
	if ((status = lag_api->get_lag_attribute(lag_oid[1], 1, lag2_port_list_attr)) != SAI_STATUS_SUCCESS) {
		printf("Failed to get LAG #2 port list\n");
		goto cleanup;
	}

	if (!check_lag_port_list(lag2_port_list_oids, lag2_port_list_attr[0].value.objlist.count, lag2_expected_results_test2,
		sizeof(lag2_expected_results_test2) / sizeof(lag2_expected_results_test2[0]))) {
		printf("Test #2 GET LAG #2 port list failed\n");
	}

	// Remove LAG_MEMBER #1
	if ((status = lag_api->remove_lag_member(lag_member_oid[0])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #1\n");
		goto cleanup;
	}

	// Remove LAG_MEMBER #4
	if ((status = lag_api->remove_lag_member(lag_member_oid[3])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #3\n");
		goto cleanup;
	}

	// Remove LAG #2
	if ((status = lag_api->remove_lag(lag_oid[1])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #2\n");
		goto cleanup;
	}

	// Remove LAG #1
	if ((status = lag_api->remove_lag(lag_oid[0])) != SAI_STATUS_SUCCESS) {
		printf("Failed to remove LAG_MEMBER #1\n");
		goto cleanup;
	}

	cleanup:
		switch_api->shutdown_switch(0);
	deinit:
		if ((status = sai_api_uninitialize()) != SAI_STATUS_SUCCESS) {
			printf("sai_api_uninitialize() failed, status=%d\n", status);
		}
	finish:
		return status;
}

bool check_lag_port_list(sai_object_id_t* port_list, uint32_t count_actual, const sai_object_id_t* lag_expected_port_list,
	uint32_t count_expected)
{
	bool is_equal = true;

	if (count_actual != count_expected) {
		is_equal = false;
		goto out;
	}

	for (uint32_t i = 0; i != count_expected; ++i) {
		printf("#%u: expected oid = 0x%lX oid = 0x%lX\n", i, port_list[i], lag_expected_port_list[i]);
		if (port_list[i] != lag_expected_port_list[i]) {
			is_equal = false;
			break;
		}
	}

	out:
		return is_equal;
}
