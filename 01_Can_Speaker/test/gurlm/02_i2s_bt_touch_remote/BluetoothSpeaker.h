// Copyright 2018 Evandro Luis Copercini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _BLUETOOTH_SERIAL_H_
#define _BLUETOOTH_SERIAL_H_

#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#include "Arduino.h"
#include "Stream.h"

class BluetoothSpeaker: public Stream
{
    public:

        BluetoothSpeaker(void);
        ~BluetoothSpeaker(void);

        bool begin(void);
        int available(void);
        int peek(void);
        int read(void);
        size_t write(uint8_t c);
        void flush();
		void remoteControl(uint8_t tl, uint8_t key_code, uint8_t key_state);
        void end(void);

    private:
        String local_name;

};

#endif

#endif
