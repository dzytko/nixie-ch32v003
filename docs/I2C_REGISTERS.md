| Address | Name             | Bits                | Type | Description                                        |
|---------|------------------|---------------------|------|----------------------------------------------------|
| 0x01    | FW Version major | Major <7:0>         | RO   | Major part of the version                          |
| 0x02    | FW Version minor | Minor <7:0>         | RO   | Minor part of the version                          |
| 0x03    | FW Version patch | Patch <7:0>         | RO   | Patch part of the version                          |
| 0x04    | Display          | Display value <4:0> | RW   | Value to display                                   |
| 0x05    | Display clear    | Clear display <0>   | WO   | Write 1 to clear the display                       |
| 0x06    | Voltage          | Voltage <4:0>       | RW   | Write: target voltage, <br/> Read: current voltage |
| 0x07    | Duty             | Duty <7:0>          | RO   | Current duty cycle                                 |
| 0x08    | Flyback ON/OFF   | Enable flyback <0>  | RW   | Enable/disable flyback converter                   |

