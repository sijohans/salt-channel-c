#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "salt.h"

void randombytes(unsigned char *p_bytes, unsigned long long length)
{
    (void) p_bytes;
    (void) length;
}

salt_ret_t my_write(salt_io_channel_t *p_wchannel)
{
    (void) p_wchannel;
    return SALT_SUCCESS;
}

salt_ret_t my_read(salt_io_channel_t *p_rchannel)
{
    static uint8_t i = 0;
    static uint32_t size;

    static uint8_t buf[SALT_HNDSHK_BUFFER_SIZE];

    switch (i) {
        case 0:
            size = read(0, buf, SALT_HNDSHK_BUFFER_SIZE);
            p_rchannel->size = 4;
            memcpy(p_rchannel->p_data, &size, 4);
            break;
        case 1:
            memcpy(p_rchannel->p_data, buf, p_rchannel->size_expected);
            p_rchannel->size = p_rchannel->size_expected;
            break;
        default:
            return SALT_ERROR;
    }

    i++;

    return SALT_SUCCESS;
}

int main(void) {

    salt_channel_t channel;
    salt_ret_t ret;
    uint8_t hndsk_buffer[SALT_HNDSHK_BUFFER_SIZE];
    memset(hndsk_buffer, 0xcc, SALT_HNDSHK_BUFFER_SIZE);
    uint8_t sig[64];

    ret = salt_create(&channel, SALT_CLIENT, my_write, my_read, NULL);
    ret = salt_set_signature(&channel, sig);
    ret = salt_init_session(&channel, hndsk_buffer, SALT_HNDSHK_BUFFER_SIZE);
    salt_protocols_t host_protocols;
    ret = salt_a1a2(&channel, hndsk_buffer, SALT_HNDSHK_BUFFER_SIZE, &host_protocols);

    if (ret == SALT_ERROR) {
        return 0;
    }

    for (uint8_t i = 0; i < host_protocols.count; i+= 2) {
        if (memcmp("Echo------",
            host_protocols.p_protocols[i+1], sizeof(salt_protocol_t)) == 0) {
            return 0;
        }
    }

    return 0;
}