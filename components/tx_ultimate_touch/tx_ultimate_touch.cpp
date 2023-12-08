#include "esphome/core/log.h"
#include "tx_ultimate_touch.h"

namespace esphome
{
    namespace tx_ultimate_touch
    {
        static const char *TAG = "tx_ultimate_touch";

        void TxUltimateTouch::setup()
        {
            this->frame_end_pos_ = 64;
            this->read_pos_ = 0;
            ESP_LOGI("log", "%s", "Tx Ultimate Touch is initialized");
        }

        void TxUltimateTouch::loop()
        {
            if(this->available()) {
                if (this->read_pos_ == sizeof(this->read_buffer_))
                {
                    // check if buffer has shifted
                    ESP_LOGW(TAG, "frame overflow, resetting");
                    this->read_pos_ = 0;
                    this->frame_size_ = 0;
                    this->frame_end_pos_ = 64;
                }

                // read byte from uart
                this->read_byte(&this->read_buffer_[this->read_pos_]);
                //ESP_LOGD("wurst", "> %d - 0x%02X", this->read_pos_, this->read_buffer_[this->read_pos_]);

                // Get packet size
                if (this->read_buffer_[this->read_pos_ - 4] == 0xAA && this->read_buffer_[this->read_pos_ - 3] == 0x55 && this->read_buffer_[this->read_pos_ - 2] == 0x01 && this->read_buffer_[this->read_pos_ - 1] == 0x02) {
                    this->frame_size_ = this->read_buffer_[this->read_pos_];
                    // end pos is current pos + framesize + 2 Byte CRC
                    this->frame_end_pos_ = this->read_pos_ + this->frame_size_ + 2;
                }

                // end frame
                if (this->read_pos_ == this->frame_end_pos_) {
                    handle_packet();

                    // reset
                    this->read_pos_ = 0;
                    this->frame_size_ = 0;
                    this->frame_end_pos_ = 64;
                } else {
                    this->read_pos_++;
                }
            }
        }

        void TxUltimateTouch::handle_packet()
        {
            int p1 = this->read_pos_ - 2 - this->frame_size_ + 1;
            int p2 = this->read_pos_ - 2 - this->frame_size_ + 2;
            int p3 = this->read_pos_ - 2 - this->frame_size_ + 3;
            int p1v = this->read_buffer_[p1];

            /*
            ESP_LOGD("wurst", "----------------------");
            ESP_LOGD("wurst", "Length: 0x%02X", this->frame_size_);
            ESP_LOGD("wurst", "p1: %d - 0x%02X", p1, p1v);
            if (this->frame_size_ >= 0x02) {
                ESP_LOGD("wurst", "p2: %d - 0x%02X", p2, this->read_buffer_[p2]);
            }
            if (this->frame_size_ >= 0x03) {
                ESP_LOGD("wurst", "p3: %d - 0x%02X", p3, this->read_buffer_[p3]);
            }
            */

            // release
            if (this->frame_size_ == 0x01) {
                int pos = this->read_buffer_[p1];
                if (pos < 0x0B) {
                    TouchPoint tp = get_touch_point(this->frame_size_, pos);
                    ESP_LOGD(TAG, "Release (pos=%d)", tp.pos);
                    this->release_trigger_.trigger(tp);
                } else if (pos == 0x0B) {
                    ESP_LOGD(TAG, "Full Touch Release");
                    TouchPoint tp = get_touch_point(this->frame_size_, pos);
                    this->full_touch_release_trigger_.trigger(tp);
                } else {
                    TouchPoint tp = get_touch_point(this->frame_size_, pos - 0x10);
                    ESP_LOGD(TAG, "Long Press Release (pos=%d)", tp.pos);
                    this->long_touch_release_trigger_.trigger(tp);
                }
            // press
            } else if (this->frame_size_ == 0x02) {
                int p2 = this->read_pos_ - 2 - this->frame_size_ + 2;
                TouchPoint tp = get_touch_point(this->frame_size_, this->read_buffer_[p2]);
                ESP_LOGD(TAG, "Press (pos=%d)", tp.pos);
                this->touch_trigger_.trigger(tp);
            } else if (this->frame_size_ == 0x03) {
                int direction = this->read_buffer_[p1];
                if (direction == 0x0C) {
                    TouchPoint tp = get_touch_point(this->frame_size_, this->read_buffer_[p2]);
                    ESP_LOGD(TAG, "Swipe Right (pos=%d)", tp.pos);
                    this->swipe_trigger_right_.trigger(tp);
                    ESP_LOGD("wurst", "Length: 0x%02X", this->frame_size_);
                    ESP_LOGD("wurst", "p1: %d - 0x%02X", p1, p1v);
                    if (this->frame_size_ >= 0x02) {
                        ESP_LOGD("wurst", "p2: %d - 0x%02X", p2, this->read_buffer_[p2]);
                    }
                    if (this->frame_size_ >= 0x03) {
                        ESP_LOGD("wurst", "p3: %d - 0x%02X", p3, this->read_buffer_[p3]);
                    }
                } else if (direction == 0x0D) {
                    TouchPoint tp = get_touch_point(this->frame_size_, this->read_buffer_[p2]);
                    ESP_LOGD(TAG, "Swipe Left (pos=%d)", tp.pos);
                    this->swipe_trigger_left_.trigger(tp);
                    ESP_LOGD("wurst", "Length: 0x%02X", this->frame_size_);
                    ESP_LOGD("wurst", "p1: %d - 0x%02X", p1, p1v);
                    if (this->frame_size_ >= 0x02) {
                        ESP_LOGD("wurst", "p2: %d - 0x%02X", p2, this->read_buffer_[p2]);
                    }
                    if (this->frame_size_ >= 0x03) {
                        ESP_LOGD("wurst", "p3: %d - 0x%02X", p3, this->read_buffer_[p3]);
                    }
                } else {
                    ESP_LOGW("wurst", "--> unknown");
                }
            } else {
                ESP_LOGW("wurst", "----------------------");
                ESP_LOGW("wurst", "Telegram unknown");
                ESP_LOGW("wurst", "Length: 0x%02X", this->frame_size_);
                //ESP_LOGW("wurst", "p1: %d - 0x%02X", p1, p1v);
                //ESP_LOGW("wurst", "p2: %d - 0x%02X", p2, p2v);
            }
            ESP_LOGD("wurst", "\n");
        }

        void TxUltimateTouch::dump_config()
        {
            ESP_LOGCONFIG(TAG, "Tx Ultimate Touch");
        }

        TouchPoint TxUltimateTouch::get_touch_point(int state, int pos)
        {
            TouchPoint tp;

            tp.pos = pos;
            tp.state = state;

            return tp;
        }

    } // namespace tx_ultimate_touch
} // namespace esphome
