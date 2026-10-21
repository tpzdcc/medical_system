#include "doctor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DOC_SQL_BUF 2048
#define DOC_ESC_BUF 224
static void doctor_fill_row(DB_RES* res, DB_ROW* row, Doctor* d) 
{
    memset(d, 0, sizeof(*d));
    snprintf(d->doctor_id, sizeof(d->doctor_id), "%s", db_row_str(res, row, 0));
    snprintf(d->name,      sizeof(d->name),      "%s", db_row_str(res, row, 1));
    snprintf(d->gender,    sizeof(d->gender),    "%s", db_row_str(res, row, 2));
    snprintf(d->title,     sizeof(d->title),     "%s", db_row_str(res, row, 3));
    snprintf(d->dept,      sizeof(d->dept),      "%s", db_row_str(res, row, 4));
    snprintf(d->phone,     sizeof(d->phone),     "%s", db_row_str(res, row, 5));
    snprintf(d->username,  sizeof(d->username),  "%s", db_row_str(res, row, 6));
    snprintf(d->password,  sizeof(d->password),  "%s", db_row_str(res, row, 7));
    d->status = db_row_int(res, row, 8);
}
int doctor_add(DB_HANDLE* db, Doctor* d) 
{
    char buf[DOC_SQL_BUF];
    char esc_name[DOC_ESC_BUF];
    char esc_gender[DOC_ESC_BUF];
    char esc_title[DOC_ESC_BUF];
    char esc_dept[DOC_ESC_BUF];
    char esc_phone[DOC_ESC_BUF];
    char esc_username[DOC_ESC_BUF];
    char esc_password[DOC_ESC_BUF];
    char* lit_name;
    char* lit_gender;
    char* lit_title;
    char* lit_dept;
    char* lit_phone;
    char* lit_username;
    char* lit_password;
    DB_RES* res;
    DB_ROW* row;
    char idtmp[16];
    int  idlen;
    int  seq = 0;
    if (!db || !db->is_connected) 
    {
        return -1;
    }
    if (!d) 
    {
        return -2;
    }
    res = db_query(db,"SELECT MAX(CAST(SUBSTRING(doctor_id, 2) AS UNSIGNED))"
                      "FROM t_doctor");
    if (!res) 
    {
        return -2;
    }
    row = db_fetch_row(res);
    if (row && row->fields[0]) seq = atoi(row->fields[0]);
    db_res_free(res);
    seq++;
    idlen = snprintf(idtmp, sizeof(idtmp), "D%03d", seq);
    if (idlen < 0 || (size_t)idlen >= sizeof(d->doctor_id)) 
    {
        return -2;
    }
    memcpy(d->doctor_id, idtmp, (size_t)idlen + 1);
    snprintf(d->username, sizeof(d->username), "%s", d->doctor_id);
    lit_name     = db_escape_literal(db, d->name,     esc_name,     sizeof(esc_name));
    lit_gender   = db_escape_literal(db, d->gender,   esc_gender,   sizeof(esc_gender));
    lit_title    = db_escape_literal(db, d->title,    esc_title,    sizeof(esc_title));
    lit_dept     = db_escape_literal(db, d->dept,     esc_dept,     sizeof(esc_dept));
    lit_phone    = db_escape_literal(db, d->phone,    esc_phone,    sizeof(esc_phone));
    lit_username = db_escape_literal(db, d->username, esc_username, sizeof(esc_username));
    lit_password = db_escape_literal(db, d->password, esc_password, sizeof(esc_password));
    if (!lit_name || !lit_gender || !lit_title || !lit_dept ||!lit_phone || !lit_username || !lit_password) 
    {
        return -2;
    }
    snprintf(buf, sizeof(buf),
             "INSERT INTO t_doctor "
             "(doctor_id, name, gender, title, dept, phone, username, password, status) "
             "VALUES ('%s', %s, %s, %s, %s, %s, %s, %s, 1)",
             d->doctor_id, lit_name, lit_gender, lit_title, lit_dept,
             lit_phone, lit_username, lit_password);
    if (db_execute(db, buf) < 0) 
    {
        if (mysql_errno(db->mysql) == 1062) 
        {
            return -3;
        }
        return -2;
    }
    return 0;
}
int doctor_delete(DB_HANDLE* db, const char* doctor_id) 
{
    char buf[512];
    char esc[64];
    int  affected;
    if (!db || !db->is_connected) 
    {
        return -1;
    }
    if (!doctor_id || !doctor_id[0])
    {
        return -4;
    }
    if (!db_escape_literal(db, doctor_id, esc, sizeof(esc))) 
    {
        return -2;
    }
    snprintf(buf, sizeof(buf),
             "UPDATE t_doctor SET status = 0 "
             "WHERE doctor_id = %s AND status = 1", esc);
    affected = db_execute(db, buf);
    if (affected < 0)
    { 
        return -2;
    }
    if (affected == 0) 
    {
        return -4;
    }
    return 0;
}
int doctor_update(DB_HANDLE* db, const Doctor* d) 
{
    char buf[DOC_SQL_BUF];
    char esc_id[64];
    char esc_name[DOC_ESC_BUF];
    char esc_gender[DOC_ESC_BUF];
    char esc_title[DOC_ESC_BUF];
    char esc_dept[DOC_ESC_BUF];
    char esc_phone[DOC_ESC_BUF];
    char esc_password[DOC_ESC_BUF];
    char* lit_id;
    char* lit_name;
    char* lit_gender;
    char* lit_title;
    char* lit_dept;
    char* lit_phone;
    char* lit_password;
    DB_RES* res;
    DB_ROW* row;
    int exists;
    if (!db || !db->is_connected) 
    {
        return -1;
    }
    if (!d || !d->doctor_id[0]) 
    {
        return -4;
    }
    lit_id = db_escape_literal(db, d->doctor_id, esc_id, sizeof(esc_id));
    if (!lit_id) 
    {
        return -2;
    }
    snprintf(buf, sizeof(buf), "SELECT doctor_id FROM t_doctor WHERE doctor_id = %s", lit_id);
    res = db_query(db, buf);
    if (!res) 
    {
        return -2;
    }
    row = db_fetch_row(res);
    exists = (row != NULL);
    db_res_free(res);
    if (!exists) 
    {
        return -4;
    }
    lit_name     = db_escape_literal(db, d->name,     esc_name,     sizeof(esc_name));
    lit_gender   = db_escape_literal(db, d->gender,   esc_gender,   sizeof(esc_gender));
    lit_title    = db_escape_literal(db, d->title,    esc_title,    sizeof(esc_title));
    lit_dept     = db_escape_literal(db, d->dept,     esc_dept,     sizeof(esc_dept));
    lit_phone    = db_escape_literal(db, d->phone,    esc_phone,    sizeof(esc_phone));
    lit_password = db_escape_literal(db, d->password, esc_password, sizeof(esc_password));
    if (!lit_name || !lit_gender || !lit_title ||
        !lit_dept || !lit_phone || !lit_password) 
    {
        return -2;
    }
    snprintf(buf, sizeof(buf),
             "UPDATE t_doctor SET name = %s, gender = %s, title = %s, "
             "dept = %s, phone = %s, "
             "password = COALESCE(NULLIF(%s, ''), password) "
             "WHERE doctor_id = %s",
             lit_name, lit_gender, lit_title, lit_dept, lit_phone,
             lit_password, lit_id);
    if (db_execute(db, buf) < 0) 
    {
        return -2;
    }
    return 0;
}
int doctor_query_by_id(DB_HANDLE* db, const char* doctor_id, Doctor* out) {
    char buf[512];
    char esc[64];
    DB_RES* res;
    DB_ROW* row;
    if (!db || !db->is_connected) 
    {
        return -1;
    }
    if (!doctor_id || !doctor_id[0] || !out) 
    {
        return -4;
    }
    if (!db_escape_literal(db, doctor_id, esc, sizeof(esc))) 
    {
        return -2;
    }
    snprintf(buf, sizeof(buf),
             "SELECT doctor_id, name, gender, title, dept, phone, username, "
             "password, status "
             "FROM t_doctor WHERE doctor_id = %s", esc);
    res = db_query(db, buf);
    if (!res) 
    {
        return -2;
    }
    row = db_fetch_row(res);
    if (!row) 
    {
        db_res_free(res);
        return -4;
    }
    doctor_fill_row(res, row, out);
    db_res_free(res);
    return 0;
}
int doctor_query_all(DB_HANDLE* db, Doctor** out_list, int* out_count) {
    DB_RES*  res;
    DB_ROW*  row;
    Doctor*  arr = NULL;
    int      n = 0;
    if (out_list)  *out_list  = NULL;
    if (out_count) *out_count = 0;
    if (!db || !db->is_connected) 
    {
        return -1;
    }
    if (!out_list || !out_count) 
    {
        return -2;
    }
    res = db_query(db,
                   "SELECT doctor_id, name, gender, title, dept, phone, "
                   "username, password, status "
                   "FROM t_doctor WHERE status = 1 ORDER BY doctor_id");
    if (!res) 
    {
        return -2;
    }
    if (res->row_cnt > 0)
     {
        arr = (Doctor*)malloc(sizeof(Doctor) * (size_t)res->row_cnt);
        if (!arr) 
        { db_res_free(res); 
            return -2; 
        }
        while ((row = db_fetch_row(res)) != NULL) 
        {
            if (n >= res->row_cnt) 
            {
                break;
            }
            doctor_fill_row(res, row, &arr[n]);
            n++;
        }
    }
    db_res_free(res);
    if (n == 0) 
    { 
        free(arr); arr = NULL; 
    }
    *out_list  = arr;
    *out_count = n;
    return 0;
}
void doctor_list_free(Doctor* list) 
{
    free(list);
}