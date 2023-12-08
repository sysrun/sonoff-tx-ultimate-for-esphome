#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/light/addressable_light.h"

namespace esphome
{
    namespace tx_ultimate_touch
    {
        const static uint8_t TOUCH_STATE_RELEASE = 0x01;
        const static uint8_t TOUCH_STATE_PRESS = 0x02;
        const static uint8_t TOUCH_STATE_SWIPE = 0x03;

        const static uint8_t TOUCH_STATE_ALL_FIELDS = 0x0B;

        const static uint8_t TOUCH_STATE_SWIPE_RIGHT = 0x0C;
        const static uint8_t TOUCH_STATE_SWIPE_LEFT = 0x0D;

        struct TouchPoint
        {
            int8_t pos = -1;
            int8_t state = -1;
        };

        class TxUltimateTouch : public uart::UARTDevice, public Component
        {
        public:
            Trigger<TouchPoint> *get_touch_trigger() { return &this->touch_trigger_; }
            Trigger<TouchPoint> *get_release_trigger() { return &this->release_trigger_; }
            Trigger<TouchPoint> *get_swipe_left_trigger() { return &this->swipe_trigger_left_; }
            Trigger<TouchPoint> *get_swipe_right_trigger() { return &this->swipe_trigger_right_; }
            Trigger<TouchPoint> *get_full_touch_release_trigger() { return &this->full_touch_release_trigger_; }
            Trigger<TouchPoint> *get_long_touch_release_trigger() { return &this->long_touch_release_trigger_; }

            void set_uart_component(esphome::uart::UARTComponent *uart_component)
            {
                this->set_uart_parent(uart_component);
            }

            void setup() override;
            void loop() override;
            void dump_config() override;

        protected:
            uint8_t read_pos_{0};
            uint8_t frame_size_{0};
            uint8_t frame_end_pos_{0};
            //uint8_t read_buffer_[0xFF]{0};
            uint8_t read_buffer_[64]{0};

            void handle_packet();

            TouchPoint get_touch_point(int state, int pos);

            Trigger<TouchPoint> touch_trigger_;
            Trigger<TouchPoint> release_trigger_;
            Trigger<TouchPoint> swipe_trigger_left_;
            Trigger<TouchPoint> swipe_trigger_right_;
            Trigger<TouchPoint> full_touch_release_trigger_;
            Trigger<TouchPoint> long_touch_release_trigger_;

        }; // class TxUltimateTouch

    } // namespace tx_ultimate_touch
} // namespace esphome
