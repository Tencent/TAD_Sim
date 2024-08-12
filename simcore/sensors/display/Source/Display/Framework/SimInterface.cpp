// Fill out your copyright notice in the Description page of Project Settings.

#include "SimInterface.h"

void ISimInterface::SimInput(bool _bNeedReturn /*= true*/)
{
}

void ISimInterface::SimInput(const FSimData& _Output, bool _bNeedReturn /*= true*/)
{
}

void ISimInterface::SimInput(const FLocalData& _Input)
{
}

void ISimInterface::SimCacul(FLocalData& _Output)
{
}

bool ISimInterface::IsComplete()
{
    return true;
}

void ISimInterface::SimOutput(const FLocalData& _Output)
{
}
