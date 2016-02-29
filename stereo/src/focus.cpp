/*
 * Camera focus/exposure change
 * Copyright (C) 2015 Victor H. Schulz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*=====================================- HEADER -======================================*/

#include "focus.h"
#include <iostream>

using namespace std;

/*-------------------------------------------------------------------------------------*/


/*=====================================- CAMERA -======================================*/

Camera::Camera(const char* device)
{
    int ret = c_init();
    if(ret) cerr << "Unable to c_init (%d)." << endl;
    cam_id = c_open_device(device);
    setAutoFocus(0);
    setFocus(0); //Bug fix.
    setAutoExposure(false);
}

Camera::~Camera()
{
    c_close_device(cam_id);
    c_cleanup();
}

int Camera::setAutoFocus(int val)
{
    CControlValue value;
    value.value = val;
    int ret = c_set_control(cam_id, CC_AUTO_FOCUS, &value);
    return ret;
}

int Camera::setFocus(int val)
{
    CControlValue value;
    value.value = val;
    int ret = c_set_control(cam_id, CC_FOCUS_ABSOLUTE, &value);
    return ret;
}

int Camera::setAutoExposure(bool status)
{
    CControlValue value;
    if(status)
        value.value = 3;
    else
        value.value = 1;
    int ret = c_set_control(cam_id, CC_AUTO_EXPOSURE_MODE, &value);
    return ret;
}

int Camera::setExposureTime(int val)
{
    CControlValue value;
    value.value = val;
    int ret = c_set_control(cam_id, CC_EXPOSURE_TIME_ABSOLUTE, &value);
    return ret;
}

/*-------------------------------------------------------------------------------------*/

