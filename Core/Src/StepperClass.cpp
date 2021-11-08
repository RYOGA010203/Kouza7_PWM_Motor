
#include <StepperClass.h>

// 初期化①コンストラクタ
StepperClass::StepperClass()
{
	// 以下の変数の単位はpulse(整数)
	target_vel = 0;
	current_vel = 0;
	target_speed = 0;
	current_speed = 0;
	target_pos = 0;
	current_pos = 0;
	// その他
	rotation_direction = 0;
	is_enabled = 0;
	tim_prescaler_value = max_prescaler_value;
}

// 有効化関数
void StepperClass::enable()
{
	HAL_TIM_Base_Start_IT(HTIM);
	HAL_TIM_PWM_Start(HTIM, TIM_CHANNEL_1);

	HAL_GPIO_WritePin(enable_GPIOx, enable_GPIO_Pin, GPIO_PIN_SET);
	is_enabled = 1;

	target_vel = 0;
	current_vel = 0;
	target_speed = 0;
	current_speed = 0;
}

// 無効化関数
void StepperClass::disable()
{
	HAL_TIM_PWM_Stop_IT(HTIM, TIM_CHANNEL_1);

	HAL_GPIO_WritePin(enable_GPIOx, enable_GPIO_Pin, GPIO_PIN_RESET);
	is_enabled = 0;
}

// 初期化②初期化関数、使用するタイマを引数にしている
void StepperClass::initialize(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
	{
		direction_GPIOx 	= GPIOA;
		direction_GPIO_Pin	= GPIO_PIN_9;
		enable_GPIOx		= GPIOA;
		enable_GPIO_Pin		= GPIO_PIN_10;
	}
	else if(htim->Instance == TIM2)
	{
		direction_GPIOx 	= GPIOA;
		direction_GPIO_Pin	= GPIO_PIN_1;
		enable_GPIOx		= GPIOB;
		enable_GPIO_Pin		= GPIO_PIN_10;
	}
	else if(htim->Instance == TIM3)
	{
		direction_GPIOx 	= GPIOA;
		direction_GPIO_Pin	= GPIO_PIN_7;
		enable_GPIOx		= GPIOB;
		enable_GPIO_Pin		= GPIO_PIN_0;
	}

	HTIM = htim;

	__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 36 - 1);
}

// 周期関数：いろいろな関数を周期的に呼び出す
void StepperClass::update()
{
	if(is_enabled == 1)
	{
		update_current_vel();
		update_current_speed();
		update_tim_prescaler_value();
		update_PWM_period();
		update_current_vel();
	}
	else
	{

	}
}

// 現在速度を目標値に少し近づける = 台形制御
void StepperClass::update_current_vel()
{
	int32_t error = target_vel - current_vel;

	if (error > (int32_t)max_acceleration)
	{
		current_vel = current_vel + (int32_t)max_acceleration;
	}
	else if(error < (int32_t)( max_acceleration*(-1) ))
	{
		current_vel = current_vel - (int32_t)max_acceleration;
	}
	else
	{
		current_vel = target_vel;
	}
}

// 速度の符号（プラス・マイナス）を回転方向に、大きさを速さに代入
void StepperClass::update_current_speed()
{
	if(current_vel > 0)
	{
		current_speed = (uint32_t)current_vel;
		rotation_direction = 1;
		HAL_GPIO_WritePin(direction_GPIOx, direction_GPIO_Pin, GPIO_PIN_SET);
	}
	else if(current_vel < 0)
	{
		current_speed = (uint32_t)( current_vel*(-1) );
		rotation_direction = 0;
		HAL_GPIO_WritePin(direction_GPIOx, direction_GPIO_Pin, GPIO_PIN_RESET);
	}
	else
	{
		current_speed = 0;
	}
}

// 速さから、タイマに設定すべきプリスケーラの値を計算
void StepperClass::update_tim_prescaler_value()
{
	if (current_speed < (tim_frequency / max_prescaler_value))
	{
		tim_prescaler_value = (uint16_t)(max_prescaler_value);
	}
	else if(current_speed > (tim_frequency / min_prescaler_value))
	{
		tim_prescaler_value = (uint16_t)(min_prescaler_value);
	}
	else
	{
		tim_prescaler_value = (uint16_t)(tim_frequency / current_speed);
	}
}

// プリスケーラをタイマに設定
void StepperClass::update_PWM_period()
{
	__HAL_TIM_SET_PRESCALER(HTIM, tim_prescaler_value);
}
