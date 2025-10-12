class OtherMenu;

class BatteryOverlay : public tsl::Gui {
private:
    char Battery_c[1024];
    bool skipOnce = true;
    bool runOnce = true;

    const char* getChargerTypeString(BatteryChargeInfoFieldsChargerType chargerType) {
        switch(chargerType) {
            case None:
                return "无";
            case PD:
                return "PD充电器";
            case TypeC_1500mA:
                return "Type-C 1.5A";
            case TypeC_3000mA:
                return "Type-C 3A";
            case DCP:
                return "专用充电端口(DCP)";
            case CDP:
                return "带数据传输充电端口(CDP)";
            case SDP:
                return "标准数据端口(SDP)";
            case Apple_500mA:
                return "Apple 0.5A";
            case Apple_1000mA:
                return "Apple 1A";
            case Apple_2000mA:
                return "Apple 2A";
            default:
                return "未知";
        }
    }

public:
    BatteryOverlay() {
        disableJumpTo = true;
        mutexInit(&mutex_BatteryChecker);
        StartBatteryThread();
    }
    ~BatteryOverlay() {
        CloseBatteryThread();
        fixForeground = true;
    }

    virtual tsl::elm::Element* createUI() override {

        auto* Status = new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawString("电池/充电状态:", false, 20, 120, 20, 0xFFFF);
            renderer->drawString(Battery_c, false, 20, 155, 18, 0xFFFF);
        });

        tsl::elm::HeaderOverlayFrame* rootFrame = new tsl::elm::HeaderOverlayFrame("状态监控", APP_VERSION, true);
        rootFrame->setContent(Status);

        return rootFrame;
    }

    virtual void update() override {

        ///Battery

        mutexLock(&mutex_BatteryChecker);
        char tempBatTimeEstimate[8] = "--:--";
        if (batTimeEstimate >= 0) {
            snprintf(&tempBatTimeEstimate[0], sizeof(tempBatTimeEstimate), "%d:%02d", batTimeEstimate / 60, batTimeEstimate % 60);
        }

        const BatteryChargeInfoFieldsChargerType ChargerConnected = hosversionAtLeast(17,0,0) ? ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields)->ChargerType : _batteryChargeInfoFields.ChargerType;
        const int32_t ChargerVoltageLimit = hosversionAtLeast(17,0,0) ? ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields)->ChargerVoltageLimit : _batteryChargeInfoFields.ChargerVoltageLimit;
        const int32_t ChargerCurrentLimit = hosversionAtLeast(17,0,0) ? ((BatteryChargeInfoFields17*)&_batteryChargeInfoFields)->ChargerCurrentLimit : _batteryChargeInfoFields.ChargerCurrentLimit;

        if (ChargerConnected)
            snprintf(Battery_c, sizeof Battery_c,
                "电池实际容量：%.0f mAh\n"
                "电池设计容量：%.0f mAh\n"
                "电池温度：%.1f\u2103\n"
                "电池原始电量：%.1f%%\n"
                "电池寿命：%.1f%%\n"
                "电池电压(平均%ds)：%.0f mV\n"
                "电池电流(平均%ss)：%+.0f mA\n"
                "电池功率%s：%+.3f W\n"
                "电池剩余时间：%s\n"
                "输入电流限制：%d mA\n"
                "VBUS 电流限制：%d mA\n" 
                "充电电压限制：%d mV\n"
                "充电电流限制：%d mA\n"
                "充电器类型：%s\n"
                "充电器最大电压：%u mV\n"
                "充电器最大电流：%u mA",
                actualFullBatCapacity,
                designedFullBatCapacity,
                (float)_batteryChargeInfoFields.BatteryTemperature / 1000,
                (float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
                (float)_batteryChargeInfoFields.BatteryAge / 1000,
                batteryFiltered ? 45 : 5, batVoltageAvg,
                batteryFiltered ? "11.25" : "5", batCurrentAvg,
                batteryFiltered ? "" : "(平均5s)", PowerConsumption, 
                tempBatTimeEstimate,
                _batteryChargeInfoFields.InputCurrentLimit,
                _batteryChargeInfoFields.VBUSCurrentLimit,
                _batteryChargeInfoFields.ChargeVoltageLimit,
                _batteryChargeInfoFields.ChargeCurrentLimit,
                getChargerTypeString(ChargerConnected),
                ChargerVoltageLimit,
                ChargerCurrentLimit
            );
        else
            snprintf(Battery_c, sizeof Battery_c,
                "电池实际容量：%.0f mAh\n"
                "电池设计容量：%.0f mAh\n"
                "电池温度：%.1f\u2103\n"
                "电池原始电量：%.1f%%\n"
                "电池寿命：%.1f%%\n"
                "电池电压(平均%ds)：%.0f mV\n"
                "电池电流(平均%ss)：%+.0f mA\n"
                "电池功率%s：%+.3f W\n"
                "电池剩余时间：%s",
                actualFullBatCapacity,
                designedFullBatCapacity,
                (float)_batteryChargeInfoFields.BatteryTemperature / 1000,
                (float)_batteryChargeInfoFields.RawBatteryCharge / 1000,
                (float)_batteryChargeInfoFields.BatteryAge / 1000,
                batteryFiltered ? 45 : 5, batVoltageAvg,
                batteryFiltered ? "11.25" : "5", batCurrentAvg,
                batteryFiltered ? "" : "(平均5s)", PowerConsumption, 
                tempBatTimeEstimate
            );
        mutexUnlock(&mutex_BatteryChecker);
        
        //static bool skipOnce = true;
    
        if (!skipOnce) {
            //static bool runOnce = true;
            if (runOnce) {
                isRendering = true;
                leventClear(&renderingStopEvent);
                runOnce = false;  // Add this to prevent repeated calls
            }
        } else {
            skipOnce = false;
        }
    }
    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (keysDown & KEY_B) {
            isRendering = false;
            leventSignal(&renderingStopEvent);
            skipOnce = true;
            runOnce = true;
            lastSelectedItem = "Battery/Charger";
            lastMode = "";
            tsl::swapTo<OtherMenu>();
            return true;
        }
        return false;
    }
};
