// ========================================================================================
//	EARTH_PT3.h
//	バージョン 0.96 (2012.07.09)
//	※バージョン 1.0 になるまではインターフェースが変更になる可能性があります。
//	※バージョン 0.91～0.96 までのインターフェースは下位互換になっています。
// ========================================================================================

#ifndef _EARTH_PT3_H
#define _EARTH_PT3_H

#include "Prefix.h"

namespace EARTH {
namespace PT {
	class Device;
	class Device_;

	// +------------+
	// | バスクラス |
	// +------------+
	// バス上のデバイスを列挙します。またデバイスインスタンスを生成します。
	class Bus {
	public:
		// [機能] Bus インスタンスを生成
		// [説明] 利用対象ドライバ名は "windrvr6_EARTHSOFT_PT3" です。
		// [返値] STATUS_INVALID_PARAM_ERROR → 引数 bus が NULL
		//        STATUS_WDAPI_LOAD_ERROR    → LoadLibrary(TEXT("wdapi1100.dll")) の返値が NULL
		//        STATUS_WD_DriverName_ERROR → WD_DriverName() の返値が NULL
		//        STATUS_WD_Open_ERROR       → WD_Open() でエラーが発生
		//        STATUS_WD_Version_ERROR    → WD_Version() でエラーが発生。またはバージョンが 11.1.0 でない
		//        STATUS_WD_License_ERROR    → WD_License() でエラーが発生
		typedef status (*NewBusFunction)(Bus **bus);

		// [機能] インスタンスを解放
		// [説明] delete は使えません。この関数を呼び出してください。
		// [返値] STATUS_ALL_DEVICES_MUST_BE_DELETED_ERROR → NewDevice() で生成されたデバイスが全て Delete() されていない
		virtual status Delete() = 0;

		// [機能] SDK バージョンを取得
		// [説明] バージョンが 2.0 の場合、値は 0x200 になります。
		//        上位 24 ビットが同じであればバイナリ互換になるように努めますので、
		//        ((version >> 8) == 2) であるかをチェックしてください。
		// [返値] STATUS_INVALID_PARAM_ERROR → 引数 version が NULL
		virtual status GetVersion(uint32 *version) const = 0;

		// デバイス情報
		struct DeviceInfo {
			uint32	Bus;		// PCI バス番号
			uint32	Slot;		// PCI デバイス番号
			uint32	Function;	// PCI ファンクション番号 (正常動作時は必ず 0 になります)
			uint32	PTn;		// 品番 (PT3:3)
		};

		// [機能] 認識されているデバイスのリストを取得
		// [説明] PCI バスをスキャンしてデバイスをリストアップします。
		//        deviceInfoCount は呼び出し前にデバイスの上限数を指定します。呼出し後は見つかったデバイス数を返します。
		// [返値] STATUS_INVALID_PARAM_ERROR   → 引数 deviceInfoPtr, deviceInfoCount のいずれかが NULL
		//        STATUS_WD_PciScanCards_ERROR → WD_PciScanCards でエラーが発生
		virtual status Scan(DeviceInfo *deviceInfoPtr, uint32 *deviceInfoCount) = 0;

		// [機能] デバイスインスタンスを生成する
		// [説明] デバイスリソースの排他チェックはこの関数では行われません。Device::Open() で行われます。
		//        Device_ は非公開インターフェースです。device_ は NULL にしてください。
		// [返値] STATUS_INVALID_PARAM_ERROR → 引数 deviceInfoPtr, device のいずれかが NULL
		//                                      または引数 _device が NULL でない
		virtual status NewDevice(const DeviceInfo *deviceInfoPtr, Device **device, Device_ **device_ = NULL) = 0;

	protected:
		virtual ~Bus() {}
	};

	// +----------------+
	// | デバイスクラス |
	// +----------------+
	// このインスタンス 1 つがボード 1 枚に対応しています。
	class Device {
	public:
		// ----
		// 解放
		// ----

		// [機能] インスタンスを解放
		// [説明] delete は使えません。この関数を呼び出してください。
		// [返値] STATUS_DEVICE_MUST_BE_CLOSED_ERROR → デバイスがオープン状態なのでインスタンスを解放できない
		virtual status Delete() = 0;
		
		// ------------------
		// オープン・クローズ
		// ------------------

		// [機能] デバイスのオープン
		// [説明] 以下の手順に沿って行われます。
		//        1. 既にデバイスがオープンされていないかを確認する。
		//        2. リビジョンID (コンフィギュレーション空間 アドレス 0x08) が 0x01 であるかを調べる。
		//        3. コンフィギュレーション空間のデバイス固有レジスタ領域を使って PCI バスでのビット化けがないかを確認する。
		//        4. この SDK で制御が可能な FPGA 回路のバージョンであるかを確認する。
		// [返値] STATUS_DEVICE_IS_ALREADY_OPEN_ERROR   → デバイスは既にオープンされている
		//        STATUS_WD_PciGetCardInfo_ERROR        → WD_PciGetCardInfo() でエラーが発生
		//        STATUS_WD_PciGetCardInfo_Bus_ERROR    → バス情報数が 1 以外
		//        STATUS_WD_PciGetCardInfo_Memory_ERROR → メモリ情報数が 1 以外
		//        STATUS_WD_CardRegister_ERROR          → WD_CardRegister() でエラーが発生
		//        STATUS_WD_PciConfigDump_ERROR         → WD_PciConfigDump() でエラーが発生
		//        STATUS_CONFIG_REVISION_ERROR          → リビジョンID が 0x01 でない
		//        STATUS_PCI_BUS_ERROR                  → PCI バスでのビット化けが発生
		//        STATUS_PCI_BASE_ADDRESS_ERROR         → コンフィギュレーション空間の BaseAddress0 が 0
		//        STATUS_FPGA_VERSION_ERROR             → 対応していない FPGA 回路バージョン
		//        STATUS_WD_CardCleanupSetup_ERROR      → WD_CardCleanupSetup() でエラーが発生
		//        STATUS_DCM_LOCK_TIMEOUT_ERROR         → DCM が一定時間経過後もロック状態にならない
		//        STATUS_DCM_SHIFT_TIMEOUT_ERROR        → DCM のフェーズシフトが一定時間経過後も完了しない
		virtual status Open() = 0;

		// [機能] デバイスのクローズ
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		virtual status Close() = 0;

		// ------------
		// 固定情報取得
		// ------------
		struct ConstantInfo {
			uint8	PTn;							// レジスタ 0x00 [31:24]
			uint8	Version_RegisterMap;			// レジスタ 0x00 [23:16]
			uint8	Version_FPGA;					// レジスタ 0x00 [15: 8]
			bool	CanTransportTS;					// レジスタ 0x0c [ 5]
			uint32	BitLength_PageDescriptorSize;	// レジスタ 0x0c [ 4: 0]
		};

		virtual status GetConstantInfo(ConstantInfo *) const = 0;

		// ------------
		// 電源・初期化
		// ------------

		enum LnbPower {
			LNB_POWER_OFF,	// オフ
			LNB_POWER_15V,	// 15V 出力
			LNB_POWER_11V	// 11V 出力
		};

		// [機能] LNB 電源制御
		// [説明] チューナーの電源とは独立に制御可能です。デフォルト値は LNB_POWER_OFF です。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 lnbPower が NULL (GetLnbPower のみ)
		virtual status SetLnbPower(LnbPower  lnbPower)       = 0;
		virtual status GetLnbPower(LnbPower *lnbPower) const = 0;

		// [機能] デバイスをクローズ（異常終了にともなうクローズを含む）時の LNB 電源制御
		// [説明] デフォルト値は LNB_POWER_OFF です。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR  → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR       → 引数 lnbPower が NULL (GetLnbPowerWhenClose のみ)
		//        STATUS_WD_CardCleanupSetup_ERROR → WD_CardCleanupSetup() でエラーが発生 (SetLnbPowerWhenClose のみ)
		virtual status SetLnbPowerWhenClose(LnbPower  lnbPower)       = 0;
		virtual status GetLnbPowerWhenClose(LnbPower *lnbPower) const = 0;

		// [機能] チューナー初期化
		// [説明] チューナーを初期化します。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_I2C_ERROR                → 復調IC からリードしたレジスタ値が異常
		virtual status InitTuner() = 0;

		// 受信方式
		enum ISDB {
			ISDB_S,
			ISDB_T,

			ISDB_COUNT
		};

		// [機能] チューナー省電力制御
		// [説明] チューナー初期後は省電力オンになっていますので、受信前に省電力をオフにする必要があります。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		//                                           引数 sleep が NULL (GetTunerSleep のみ)
		virtual status SetTunerSleep(ISDB isdb, uint32 index, bool  sleep)       = 0;
		virtual status GetTunerSleep(ISDB isdb, uint32 index, bool *sleep) const = 0;

		// ----------
		// 局発周波数
		// ----------

		// [機能] 局発周波数の制御
		// [説明] offset で周波数の調整が可能です。単位は ISDB-S の場合は 1kHz、ISDB-T の場合は 1/7MHz です。
		//        例えば、C24 を標準より 2MHz 高い周波数に設定するには SetFrequency(tuner, ISDB_T, 23, 7*2) とします。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		//                                           引数 channel が NULL (GetFrequency のみ)
		//        STATUS_TUNER_IS_SLEEP_ERROR     → チューナーが省電力状態のため設定不可 (SetFrequency のみ)
		virtual status SetFrequency(ISDB isdb, uint32 index, uint32  channel, sint32  offset = 0)       = 0;
		virtual status GetFrequency(ISDB isdb, uint32 index, uint32 *channel, sint32 *offset = 0) const = 0;

		// (ISDB-S)
		// +----+------+---------+ +----+------+---------+ +----+------+---------+
		// | ch | TP # | f (MHz) | | ch | TP # | f (MHz) | | ch | TP # | f (MHz) |
		// +----+------+---------+ +----+------+---------+ +----+------+---------+
		// |  0 | BS 1 | 1049.48 | | 12 | ND 2 | 1613.00 | | 24 | ND 1 | 1593.00 |
		// |  1 | BS 3 | 1087.84 | | 13 | ND 4 | 1653.00 | | 25 | ND 3 | 1633.00 |
		// |  2 | BS 5 | 1126.20 | | 14 | ND 6 | 1693.00 | | 26 | ND 5 | 1673.00 |
		// |  3 | BS 7 | 1164.56 | | 15 | ND 8 | 1733.00 | | 27 | ND 7 | 1713.00 |
		// |  4 | BS 9 | 1202.92 | | 16 | ND10 | 1773.00 | | 28 | ND 9 | 1753.00 |
		// |  5 | BS11 | 1241.28 | | 17 | ND12 | 1813.00 | | 29 | ND11 | 1793.00 |
		// |  6 | BS13 | 1279.64 | | 18 | ND14 | 1853.00 | | 30 | ND13 | 1833.00 |
		// |  7 | BS15 | 1318.00 | | 19 | ND16 | 1893.00 | | 31 | ND15 | 1873.00 |
		// |  8 | BS17 | 1356.36 | | 20 | ND18 | 1933.00 | | 32 | ND17 | 1913.00 |
		// |  9 | BS19 | 1394.72 | | 21 | ND20 | 1973.00 | | 33 | ND19 | 1953.00 |
		// | 10 | BS21 | 1433.08 | | 22 | ND22 | 2013.00 | | 34 | ND21 | 1993.00 |
		// | 11 | BS23 | 1471.44 | | 23 | ND24 | 2053.00 | | 35 | ND23 | 2033.00 |
		// +----+------+---------+ +----+------+---------+ +----+------+---------+
		// 
		// (ISDB-T)
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) |
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// |   0 |   1 |  93+1/7 | |  23 | C24 | 231+1/7 | |  46 | C47 | 369+1/7 | |  69 |  19 | 509+1/7 | |  92 |  42 | 647+1/7 |
		// |   1 |   2 |  99+1/7 | |  24 | C25 | 237+1/7 | |  47 | C48 | 375+1/7 | |  70 |  20 | 515+1/7 | |  93 |  43 | 653+1/7 |
		// |   2 |   3 | 105+1/7 | |  25 | C26 | 243+1/7 | |  48 | C49 | 381+1/7 | |  71 |  21 | 521+1/7 | |  94 |  44 | 659+1/7 |
		// |   3 | C13 | 111+1/7 | |  26 | C27 | 249+1/7 | |  49 | C50 | 387+1/7 | |  72 |  22 | 527+1/7 | |  95 |  45 | 665+1/7 |
		// |   4 | C14 | 117+1/7 | |  27 | C28 | 255+1/7 | |  50 | C51 | 393+1/7 | |  73 |  23 | 533+1/7 | |  96 |  46 | 671+1/7 |
		// |   5 | C15 | 123+1/7 | |  28 | C29 | 261+1/7 | |  51 | C52 | 399+1/7 | |  74 |  24 | 539+1/7 | |  97 |  47 | 677+1/7 |
		// |   6 | C16 | 129+1/7 | |  29 | C30 | 267+1/7 | |  52 | C53 | 405+1/7 | |  75 |  25 | 545+1/7 | |  98 |  48 | 683+1/7 |
		// |   7 | C17 | 135+1/7 | |  30 | C31 | 273+1/7 | |  53 | C54 | 411+1/7 | |  76 |  26 | 551+1/7 | |  99 |  49 | 689+1/7 |
		// |   8 | C18 | 141+1/7 | |  31 | C32 | 279+1/7 | |  54 | C55 | 417+1/7 | |  77 |  27 | 557+1/7 | | 100 |  50 | 695+1/7 |
		// |   9 | C19 | 147+1/7 | |  32 | C33 | 285+1/7 | |  55 | C56 | 423+1/7 | |  78 |  28 | 563+1/7 | | 101 |  51 | 701+1/7 |
		// |  10 | C20 | 153+1/7 | |  33 | C34 | 291+1/7 | |  56 | C57 | 429+1/7 | |  79 |  29 | 569+1/7 | | 102 |  52 | 707+1/7 |
		// |  11 | C21 | 159+1/7 | |  34 | C35 | 297+1/7 | |  57 | C58 | 435+1/7 | |  80 |  30 | 575+1/7 | | 103 |  53 | 713+1/7 |
		// |  12 | C22 | 167+1/7 | |  35 | C36 | 303+1/7 | |  58 | C59 | 441+1/7 | |  81 |  31 | 581+1/7 | | 104 |  54 | 719+1/7 |
		// |  13 |   4 | 173+1/7 | |  36 | C37 | 309+1/7 | |  59 | C60 | 447+1/7 | |  82 |  32 | 587+1/7 | | 105 |  55 | 725+1/7 |
		// |  14 |   5 | 179+1/7 | |  37 | C38 | 315+1/7 | |  60 | C61 | 453+1/7 | |  83 |  33 | 593+1/7 | | 106 |  56 | 731+1/7 |
		// |  15 |   6 | 185+1/7 | |  38 | C39 | 321+1/7 | |  61 | C62 | 459+1/7 | |  84 |  34 | 599+1/7 | | 107 |  57 | 737+1/7 |
		// |  16 |   7 | 191+1/7 | |  39 | C40 | 327+1/7 | |  62 | C63 | 465+1/7 | |  85 |  35 | 605+1/7 | | 108 |  58 | 743+1/7 |
		// |  17 |   8 | 195+1/7 | |  40 | C41 | 333+1/7 | |  63 |  13 | 473+1/7 | |  86 |  36 | 611+1/7 | | 109 |  59 | 749+1/7 |
		// |  18 |   9 | 201+1/7 | |  41 | C42 | 339+1/7 | |  64 |  14 | 479+1/7 | |  87 |  37 | 617+1/7 | | 110 |  60 | 755+1/7 |
		// |  19 |  10 | 207+1/7 | |  42 | C43 | 345+1/7 | |  65 |  15 | 485+1/7 | |  88 |  38 | 623+1/7 | | 111 |  61 | 761+1/7 |
		// |  20 |  11 | 213+1/7 | |  43 | C44 | 351+1/7 | |  66 |  16 | 491+1/7 | |  89 |  39 | 629+1/7 | | 112 |  62 | 767+1/7 |
		// |  21 |  12 | 219+1/7 | |  44 | C45 | 357+1/7 | |  67 |  17 | 497+1/7 | |  90 |  40 | 635+1/7 | +-----+-----+---------+
		// |  22 | C23 | 225+1/7 | |  45 | C46 | 363+1/7 | |  68 |  18 | 503+1/7 | |  91 |  41 | 641+1/7 |
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// 
		// C24～C27 は、ケーブルテレビ局により下記の周波数で送信されている場合があります。
		// +-----+---------+
		// | Ch. | f (MHz) |
		// +-----+---------+
		// | C24 | 233+1/7 |
		// | C25 | 239+1/7 |
		// | C26 | 245+1/7 |
		// | C27 | 251+1/7 |
		// +-----+---------+

		// ----------
		// 周波数誤差
		// ----------

		// [機能] 周波数誤差を取得
		// [説明] 値の意味は次の通りです。
		//        クロック周波数誤差: clock/100 (ppm)
		//        キャリア周波数誤差: carrier (Hz)
		//        放送波の周波数精度は十分に高い仮定すると、誤差が発生する要素として以下のようなものが考えられます。
		//        (ISDB-S) LNB での周波数変換精度 / 衛星側 PLL-IC に接続されている振動子の精度 / 復調 IC に接続されている振動子の精度
		//        (ISDB-T) 地上側 PLL-IC に接続されている振動子の精度 / 復調 IC に接続されている振動子の精度
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		//                                           引数 clock, carrier のいずれかが NULL
		//        STATUS_TUNER_IS_SLEEP_ERROR     → チューナーが省電力状態
		virtual status GetFrequencyOffset(ISDB isdb, uint32 tuner, sint32 *clock, sint32 *carrier) = 0;

		// --------
		// C/N・AGC
		// --------

		// [機能] C/N と AGC を取得
		// [説明] C/N は低レイテンシで測定できるため、アンテナの向きを調整するのに便利です。
		//        値の意味は次の通りです。
		//        C/N                : cn100/100 (dB)
		//        現在の AGC 値      : currentAgc
		//        利得最大時の AGC 値: maxAgc
		//        currentAgc の範囲は 0 から maxAgc までです。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		//                                           引数 cn100, currentAgc, maxAgc のいずれかが NULL
		//        STATUS_TUNER_IS_SLEEP_ERROR     → チューナーが省電力状態
		virtual status GetCnAgc(ISDB isdb, uint32 tuner, uint32 *cn100, uint32 *currentAgc, uint32 *maxAgc) = 0;

		// ----------------------
		// RF Level (ISDB-T のみ)
		// ----------------------
		virtual status GetRFLevel(uint32 tuner, float *level) = 0;

		// -------------------
		// TS-ID (ISDB-S のみ)
		// -------------------

		// [機能] TS-ID を設定
		// [説明] 設定値が復調IC の動作に反映されるまで時間が掛かります。
		//        GetLayerS() を呼び出す前に、GetIdS() を使って切り替えが完了したことを確認してください。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きい
		virtual status SetIdS(uint32 tuner, uint32 id) = 0;

		// [機能] 現在処理中の TS-ID を取得
		// [説明] GetLayerS() で取得できるレイヤ情報は、この関数で示される TS-ID のものになります。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きい。または引数 id が NULL
		virtual status GetIdS(uint32 tuner, uint32 *id) = 0;

		// ------------
		// エラーレート
		// ------------

		// 階層インデックス
		enum LayerIndex {
			// ISDB-S
			LAYER_INDEX_L = 0,	// 低階層
			LAYER_INDEX_H,		// 高階層

			// ISDB-T
			LAYER_INDEX_A = 0,	// A 階層
			LAYER_INDEX_B,		// B 階層
			LAYER_INDEX_C		// C 階層
		};

		// 階層数
		enum LayerCount {
			// ISDB-S
			LAYER_COUNT_S = LAYER_INDEX_H + 1,

			// ISDB-T
			LAYER_COUNT_T = LAYER_INDEX_C + 1
		};

		// エラーレート
		struct ErrorRate {
			uint32 Numerator, Denominator;
		};

		// [機能] GetInnerErrorRate() で対象となる階層を設定
		virtual status SetInnerErrorRateLayer(ISDB isdb, uint32 tuner, LayerIndex layerIndex) = 0;

		// [機能] 内符号で訂正されたエラーレートを取得
		virtual status GetInnerErrorRate(ISDB isdb, uint32 tuner, ErrorRate *errorRate) = 0;

		// [機能] リードソロモン復号で訂正されたエラーレートを取得
		// [説明] 測定に時間が掛かりますが、受信品質を正確に把握するには C/N ではなくこのエラーレートを参考にしてください。
		//        ひとつの目安として 2×10^-4 以下であれば、リードソロモン復号後にほぼエラーフリーになるといわれています。
		//        エラーレートの集計単位は次の通りです。
		//        ISDB-S: 1024 フレーム
		//        ISDB-T: 32 フレーム (モード 1,2) / 8 フレーム (モード 3)
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner, isdb, layerIndex が範囲外。または errorRate が NULL
		virtual status GetCorrectedErrorRate(ISDB isdb, uint32 tuner, LayerIndex layerIndex, ErrorRate *errorRate) = 0;

		// [機能] リードソロモン復号で訂正されたエラーレートを計算するためのエラーカウンタを初期化
		// [説明] 全階層のカウンタを初期化します。特定の階層のカウンタをリセットすることはできません。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		virtual status ResetCorrectedErrorCount(ISDB isdb, uint32 tuner) = 0;

		// [機能] リードソロモン復号で訂正しきれなかった TS パケット数を取得
		// [説明] 0xffffffff の次は 0x00000000 になります。
		//        TS パケットの 2nd Byte MSB を数えても同じ数値になります。
		//        このカウンタは DMA 転送開始時に初期化されます。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外。または引数 count が NULL
		virtual status GetErrorCount(ISDB isdb, uint32 tuner, uint32 *count) = 0;

		// --------------------------
		// TMCC・レイヤー・ロック判定
		// --------------------------

		// ISDB-S TMCC 情報
		// (参考) STD-B20 2.9 TMCC情報の構成 ～ 2.11 TMCC情報の更新
		struct TmccS {
			uint32 Indicator;	// 変更指示 (5ビット)
			uint32 Mode[4];		// 伝送モードn (4ビット)
			uint32 Slot[4];		// 伝送モードnへの割当スロット数 (6ビット)
								// [相対TS／スロット情報は取得できません]
			uint32 Id[8];		// 相対TS番号nに対するTS ID (16ビット)
			uint32 Emergency;	// 起動制御信号 (1ビット)
			uint32 UpLink;		// アップリンク制御情報 (4ビット)
			uint32 ExtFlag;		// 拡張フラグ (1ビット)
			uint32 ExtData[2];	// 拡張領域 (61ビット)
		};

		// [機能] ISDB-S の TMCC 情報を取得
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 tmcc が NULL
		virtual status GetTmccS(uint32 tuner, TmccS *tmcc) = 0;

		// ISDB-S 階層情報
		struct LayerS {
			uint32 Mode [LAYER_COUNT_S];	// 伝送モード (3ビット) 
			uint32 Count[LAYER_COUNT_S];	// ダミースロットを含めた割当スロット数 (6ビット)
		};

		// [機能] ISDB-S のレイヤ情報を取得
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 layerS が NULL
		virtual status GetLayerS(uint32 tuner, LayerS *layerS) = 0;

		// ISDB-T TMCC 情報
		// (参考) STD-B31 3.15.6 TMCC情報 ～ 3.15.6.8 セグメント数
		struct TmccT {
			uint32 System;						// システム識別 (2ビット)
			uint32 Indicator;					// 伝送パラメータ切り替え指標 (4ビット)
			uint32 Emergency;					// 緊急警報放送用起動フラグ (1ビット)
												// カレント情報
			uint32 Partial;						// 部分受信フラグ (1ビット)
												// 階層情報
			uint32 Mode      [LAYER_COUNT_T];	// キャリア変調方式 (3ビット)
			uint32 Rate      [LAYER_COUNT_T];	// 畳込み符号化率 (3ビット)
			uint32 Interleave[LAYER_COUNT_T];	// インターリーブ長 (3ビット)
			uint32 Segment   [LAYER_COUNT_T];	// セグメント数 (4ビット)
												// [ネクスト情報は取得できません]
			uint32 Phase;						// 連結送信位相補正量 (3ビット)
			uint32 Reserved;					// リザーブ (12ビット)
		};

		// [機能] ISDB-T の TMCC 情報を取得
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 tmcc が NULL
		virtual status GetTmccT(uint32 tuner, TmccT *tmcc) = 0;

		// [機能] ISDB-T ロック判定を取得
		// [説明] レイヤが存在し、なおかつそのレイヤがエラーフリーであるときに true になります。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 locked が NULL
//		virtual status GetLockedT(uint32 tuner, bool locked[LAYER_COUNT_T]) = 0;

		// [機能] ISDB-T 側の入力アンプの電源制御
		virtual status SetAmpPowerT(bool b) = 0;

		// 受信階層
		enum LayerMask {
			LAYER_MASK_NONE,

			// ISDB-S
			LAYER_MASK_L = 1 << LAYER_INDEX_L,
			LAYER_MASK_H = 1 << LAYER_INDEX_H,

			// ISDB-T
			LAYER_MASK_A = 1 << LAYER_INDEX_A,
			LAYER_MASK_B = 1 << LAYER_INDEX_B,
			LAYER_MASK_C = 1 << LAYER_INDEX_C
		};

		// [機能] 受信階層の設定
		// [説明] ISDB-S の低階層を受信しないように設定することはできません。
		// [返値] STATUS_DEVICE_IS_NOT_OPEN_ERROR → デバイスがオープンされていない
		//        STATUS_INVALID_PARAM_ERROR      → 引数 tuner が 1 より大きいか引数 isdb が範囲外
		//                                           引数 layerMask が範囲外 (SetLayerEnable のみ)
		//                                           引数 layerMask が NULL (GetLayerEnable のみ)
		virtual status SetLayerEnable(ISDB isdb, uint32 tuner, LayerMask  layerMask)       = 0;
		virtual status GetLayerEnable(ISDB isdb, uint32 tuner, LayerMask *layerMask) const = 0;

		// -------------
		// TS ピンテスト
		// -------------
		enum TsPinMode {
			TS_PIN_MODE_NORMAL,
			TS_PIN_MODE_LOW,
			TS_PIN_MODE_HIGH
		};

		struct TsPinsMode {
			TsPinMode	clock_data,
						byte,
						valid;
		};

		struct TsPinsLevel {
			bool	clock,	// トグル動作
					data,
					byte,
					valid;
		};

		virtual status SetTsPinsMode (ISDB isdb, uint32 tuner, const TsPinsMode  *mode ) = 0;
		virtual status GetTsPinsLevel(ISDB isdb, uint32 tuner,       TsPinsLevel *level) = 0;

		virtual status GetTsSyncByte(ISDB isdb, uint32 tuner, uint8 *syncByte) = 0;

		enum RamPinsMode {
			RAM_PINS_MODE_NORMAL,
			RAM_PINS_MODE_LOW,
			RAM_PINS_MODE_HIGH
		};

		virtual status SetRamPinsMode(RamPinsMode mode) = 0;

		// ------------------
		// DMA 転送用バッファ
		// ------------------

		// 次の関数は廃止されました。LockBuffer() をご利用ください。
		virtual status LockBuffer__Obsolated__(void *ptr, uint32 size, void **handle) = 0;
		virtual status UnlockBuffer(void *handle) = 0;

		struct BufferInfo {
			uint64 Address;		// 物理アドレス
			uint32 Size;		// サイズ
		};

		virtual status GetBufferInfo(void *handle, const BufferInfo **infoTable, uint32 *infoCount) = 0;

		// --------
		// DMA 転送
		// --------

		// [機能] DMA 開始・停止の制御
		// [説明] DMA 転送は全く CPU を介在することなく動作します。
		//        GetTransferEnabled() で true  が得られるときに SetTransferEnabled(true ) としたり、
		//        GetTransferEnabled() で false が得られるときに SetTransferEnabled(false) とするとエラーになります。
		//        
		//        GetTransferEnabled() で取得できる値は、単に SetTransferEnabled() で最後に設定された値と同じです。
		//        転送カウンタが 0 になるなど、ハードウェア側で DMA 転送が自動的に停止する要因がいくつかありますが、
		//        その場合でも GetTransferEnabled() で得られる値は変わりません。
		virtual status SetTransferPageDescriptorAddress(ISDB isdb, uint32 tunerIndex, uint64 pageDescriptorAddress) = 0;
		virtual status SetTransferEnabled              (ISDB isdb, uint32 tunerIndex, bool  enabled)       = 0;
		virtual status GetTransferEnabled              (ISDB isdb, uint32 tunerIndex, bool *enabled) const = 0;
		
		// resetError は TS エラーパケットのリセットです。将来的には別関数に移動します。
		virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode = false, uint16 initial = 0, bool not = false/*, bool resetError = false*/) = 0;
		
		struct TransferInfo {
			bool	Busy;
			uint32	Status;						// 4ビット
			bool	InternalFIFO_A_Overflow,
					InternalFIFO_A_Underflow;
			bool	ExternalFIFO_Overflow;
			uint32	ExternalFIFO_MaxUsedBytes;	// ExternalFIFO_Overflow が false の場合のみ有効
			bool	InternalFIFO_B_Overflow,
					InternalFIFO_B_Underflow;
		};

		virtual status GetTransferInfo(ISDB isdb, uint32 tunerIndex, TransferInfo *transferInfo) = 0;

		// ---------------------
		// 0.96 で追加された関数
		// ---------------------

		enum TransferDirection {				// DMA の転送方向
			TRANSFER_DIRECTION_WRITE = 1 << 0,	// PCI デバイスがメモリにデータを書き込む
			TRANSFER_DIRECTION_READ	 = 1 << 1,	// PCI デバイスがメモリのデータを読み込む
			TRANSFER_DIRECTION_WRITE_READ = TRANSFER_DIRECTION_WRITE | TRANSFER_DIRECTION_READ
		};

		// [機能] メモリ領域を物理メモリに固定する
		// [説明] DMA 転送用メモリ領域は DMA 転送を開始する前に物理メモリに固定する必要があります。
		//        ptr と size でメモリ領域を指定し、direction で DMA 転送方向を指定します。
		//        handle で返されたポインタは UnlockBuffer(), SyncBufferCpu(), SyncBufferDevice() で使用します。
		virtual status LockBuffer(void *ptr, uint32 size, TransferDirection direction, void **handle) = 0;

		// [機能] CPU キャッシュと DMA 転送用メモリ領域の同期を取る
		// [説明] 下記に記載する不整合が起こらないように、この関数を呼んで同期を取る必要性があります。
		//        (ケース1)
		//        1. CPU が DMA 転送用メモリ領域にデータを書き込む
		//        2. 書き込まれたデータは CPU キャッシュ上に存在するだけで DMA 転送用メモリ領域には未だ書き込まれていない
		//        3. PCI デバイスが DMA 転送用メモリ領域にデータを書き込む
		//        4. CPU がキャッシュ上に存在するデータを DMA 転送用メモリ領域に書き込む (不整合発生)
		//        ※不整合が発生しないように 2 の後にこの関数を呼んでください。
		//        
		//        (ケース2)
		//        1. PCI デバイスが DMA 転送用メモリ領域にデータを書き込む
		//        2. CPU が転送用メモリからデータを読み込む
		//        3. 読み込んだデータは CPU のキャッシュに保存される
		//        4. PCI デバイスが DMA 転送用メモリ領域に「新しい」データを書き込む
		//        5. CPU が転送用メモリからデータを読み込む
		//        6. CPU は DMA 転送用メモリ領域にある「新しい」データではなく、キャッシュされた古いデータを読み込む (不整合発生)
		//        ※不整合が発生しないように 5 の前にこの関数を呼んでください。
		virtual status SyncBufferCpu(void *handle) = 0;

		// [機能] I/O キャッシュと DMA 転送用メモリ領域の同期を取る
		// [説明] 下記に記載する不整合が起こらないように、この関数を呼んで同期を取る必要性があります。
		//        1. PCI デバイスが DMA 転送用メモリ領域にデータを書き込むためにパケットを送出する
		//        2. パケットを受け取ったデイバスは後でデータを次のデバイスに送出するため、自身でデータをキャッシュする
		//        3. よってデータは未だ DMA 転送用メモリ領域に書き込まれていない
		//        4. CPU は PCI デバイスが「転送終了（＝パケット送出終了）」であることを確認する
		//        5. DMA 転送用メモリ領域からデータを読み込む (不整合発生)
		//        ※不整合が発生しないように 5 の前にこの関数を呼んでください。
		virtual status SyncBufferIo(void *handle) = 0;
		
	protected:
		virtual ~Device() {}
	};

	enum Status {
		// エラーなし
		STATUS_OK,

		// 一般的なエラー
		STATUS_GENERAL_ERROR = (1)*0x100,
		STATUS_NOT_IMPLIMENTED,
		STATUS_INVALID_PARAM_ERROR,
		STATUS_OUT_OF_MEMORY_ERROR,
		STATUS_INTERNAL_ERROR,

		// バスクラスのエラー
		STATUS_WDAPI_LOAD_ERROR = (2)*256,	// wdapi1100.dll がロードできない
		STATUS_ALL_DEVICES_MUST_BE_DELETED_ERROR,

		// デバイスクラスのエラー
		STATUS_PCI_BUS_ERROR = (3)*0x100,
		STATUS_CONFIG_REVISION_ERROR,
		STATUS_FPGA_VERSION_ERROR,
		STATUS_PCI_BASE_ADDRESS_ERROR,
		STATUS_FLASH_MEMORY_ERROR,

		STATUS_DCM_LOCK_TIMEOUT_ERROR,
		STATUS_DCM_SHIFT_TIMEOUT_ERROR,

		STATUS_POWER_RESET_ERROR,
		STATUS_I2C_ERROR,
		STATUS_TUNER_IS_SLEEP_ERROR,

		STATUS_PLL_OUT_OF_RANGE_ERROR,
		STATUS_PLL_LOCK_TIMEOUT_ERROR,

		STATUS_VIRTUAL_ALLOC_ERROR,
		STATUS_DMA_ADDRESS_ERROR,
		STATUS_BUFFER_ALREADY_ALLOCATED_ERROR,

		STATUS_DEVICE_IS_ALREADY_OPEN_ERROR,
		STATUS_DEVICE_IS_NOT_OPEN_ERROR,

		STATUS_BUFFER_IS_IN_USE_ERROR,
		STATUS_BUFFER_IS_NOT_ALLOCATED_ERROR,

		STATUS_DEVICE_MUST_BE_CLOSED_ERROR,

		// WinDriver 関連のエラー
		STATUS_WD_DriverName_ERROR = (4)*0x100,

		STATUS_WD_Open_ERROR,
		STATUS_WD_Close_ERROR,

		STATUS_WD_Version_ERROR,
		STATUS_WD_License_ERROR,

		STATUS_WD_PciScanCards_ERROR,

		STATUS_WD_PciConfigDump_ERROR,

		STATUS_WD_PciGetCardInfo_ERROR,
		STATUS_WD_PciGetCardInfo_Bus_ERROR,
		STATUS_WD_PciGetCardInfo_Memory_ERROR,

		STATUS_WD_CardRegister_ERROR,
		STATUS_WD_CardUnregister_ERROR,

		STATUS_WD_CardCleanupSetup_ERROR,

		STATUS_WD_DMALock_ERROR,
		STATUS_WD_DMAUnlock_ERROR,

		STATUS_WD_DMASyncCpu_ERROR,
		STATUS_WD_DMASyncIo_ERROR,

		// ROM
		STATUS_ROM_ERROR = (5)*0x100,
		STATUS_ROM_TIMEOUT
	};
}
}

#endif
