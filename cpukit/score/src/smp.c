/**
 *  @file
 *
 *  @brief SMP Support
 *  @ingroup Score
 */

/*
 *  COPYRIGHT (c) 1989-2011.
 *  On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/score/smpimpl.h>
#include <rtems/score/assert.h>
#include <rtems/score/threaddispatch.h>
#include <rtems/score/threadimpl.h>
#include <rtems/config.h>

#if defined(RTEMS_DEBUG)
  #include <rtems/bspIo.h>
#endif

void _SMP_Handler_initialize( void )
{
  uint32_t max_cpus = rtems_configuration_get_maximum_processors();
  uint32_t cpu;

  for ( cpu = 0 ; cpu < max_cpus; ++cpu ) {
    Per_CPU_Control *per_cpu = _Per_CPU_Get_by_index( cpu );

    _ISR_lock_Initialize( &per_cpu->lock );
  }

  /*
   * Discover and initialize the secondary cores in an SMP system.
   */
  max_cpus = _CPU_SMP_Initialize( max_cpus );

  _SMP_Processor_count = max_cpus;
}

void _SMP_Start_multitasking_on_secondary_processor( void )
{
  Per_CPU_Control *self_cpu = _Per_CPU_Get();

  #if defined(RTEMS_DEBUG)
    printk( "Made it to %d -- ", _Per_CPU_Get_index( self_cpu ) );
  #endif

  _Per_CPU_Change_state( self_cpu, PER_CPU_STATE_READY_TO_BEGIN_MULTITASKING );

  _Per_CPU_Wait_for_state( self_cpu, PER_CPU_STATE_BEGIN_MULTITASKING );

  _Thread_Start_multitasking();
}

void _SMP_Send_message( uint32_t cpu, uint32_t message )
{
  Per_CPU_Control *per_cpu = _Per_CPU_Get_by_index( cpu );
  ISR_Level level;

  _Per_CPU_ISR_disable_and_acquire( per_cpu, level );
  per_cpu->message |= message;
  _Per_CPU_Release_and_ISR_enable( per_cpu, level );

  _CPU_SMP_Send_interrupt( cpu );
}

void _SMP_Broadcast_message( uint32_t message )
{
  uint32_t self = _SMP_Get_current_processor();
  uint32_t ncpus = _SMP_Get_processor_count();
  uint32_t cpu;

  _Assert_Thread_dispatching_repressed();

  for ( cpu = 0 ; cpu < ncpus ; ++cpu ) {
    if ( cpu != self ) {
      _SMP_Send_message( cpu, message );
    }
  }
}

void _SMP_Request_other_cores_to_perform_first_context_switch( void )
{
  uint32_t self = _SMP_Get_current_processor();
  uint32_t ncpus = _SMP_Get_processor_count();
  uint32_t cpu;

  for ( cpu = 0 ; cpu < ncpus ; ++cpu ) {
    Per_CPU_Control *per_cpu = _Per_CPU_Get_by_index( cpu );

    if ( cpu != self ) {
      _Per_CPU_Wait_for_state(
        per_cpu,
        PER_CPU_STATE_READY_TO_BEGIN_MULTITASKING
      );

      _Per_CPU_Change_state( per_cpu, PER_CPU_STATE_BEGIN_MULTITASKING );
    }
  }
}

void _SMP_Request_other_cores_to_shutdown( void )
{
  uint32_t self = _SMP_Get_current_processor();

  /*
   * Do not use _SMP_Get_processor_count() since this value might be not
   * initialized yet.  For example due to a fatal error in the middle of
   * _CPU_SMP_Initialize().
   */
  uint32_t ncpus = rtems_configuration_get_maximum_processors();

  uint32_t cpu;

  for ( cpu = 0 ; cpu < ncpus ; ++cpu ) {
    if ( cpu != self ) {
      const Per_CPU_Control *per_cpu = _Per_CPU_Get_by_index( cpu );

      if ( per_cpu->state != PER_CPU_STATE_BEFORE_INITIALIZATION ) {
        _SMP_Send_message( cpu, SMP_MESSAGE_SHUTDOWN );
      }
    }
  }
}
