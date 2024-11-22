#include <stdint.h>
#include <memory.h>

#define TLV_OVERHEAD_SIZE  2

/*Macro to Type Length Value reply
 * char* - start_ptr, IN
 * unsigned char - type, OUT
 * unsigned char - length, OUT
 * unsigned char * - tlv_ptr, OUT
 * unsigned int - total_size(excluding first 8 bytes), IN
 * */
#define ITERATE_TLV_BEGIN(start_ptr, type, length, tlv_ptr, tlv_size)           \
{                                                                               \
    unsigned int _len = 0; char _tlv_value_size = 0;                   \
    type = 0; length = 0; tlv_ptr = NULL;                                       \
    for(tlv_ptr = (char *)start_ptr +                                  \
             TLV_OVERHEAD_SIZE; _len < tlv_size;                                \
            _len += _tlv_value_size + TLV_OVERHEAD_SIZE,                        \
             tlv_ptr = (tlv_ptr + TLV_OVERHEAD_SIZE + length)){                 \
        type = *(tlv_ptr - TLV_OVERHEAD_SIZE);                                  \
        _tlv_value_size = (char)(*(tlv_ptr -                           \
            TLV_OVERHEAD_SIZE + sizeof(char)));                        \
        length = _tlv_value_size;

#define ITERATE_TLV_END(start_ptr, type, length, tlv_ptr, tlv_size)             \
    }}


static inline char*
tlv_buffer_insert_tlv(char  *buff,
                                  uint8_t tlv_no,
                                  uint8_t data_len,
                                  char  *data){

    *buff = tlv_no;
    *(buff+1) = data_len;
    if (data) {
        memcpy(buff + TLV_OVERHEAD_SIZE, data, data_len);
    }
    return buff + TLV_OVERHEAD_SIZE + data_len;
}

static inline char  *
tlv_buffer_get_particular_tlv(char  *tlv_buff, /*Input TLV Buffer*/
                      uint32_t tlv_buff_size,               /*Input TLV Buffer Total Size*/
                      uint8_t tlv_no,                            /*Input TLV Number*/
                      uint8_t *tlv_data_len){              /*Output TLV Data len*/

    char tlv_type, tlv_len, *tlv_value = NULL;
    
    ITERATE_TLV_BEGIN(tlv_buff, tlv_type, tlv_len, tlv_value, tlv_buff_size){
        
        if(tlv_type != tlv_no) continue;
        *tlv_data_len = tlv_len;
        return tlv_value;
    }ITERATE_TLV_END(tlv_buff, tlv_type, tlv_len, tlv_value, tlv_buff_size); 

    *tlv_data_len = 0;

    return NULL;
}