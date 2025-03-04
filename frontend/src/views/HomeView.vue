<template>
    <div class='home'>
        <RoomSelection class='left'/>
        <p class='right'>Right</p>
    </div>
</template>


<script setup>
    import RoomSelection from '../components/RoomSelection.vue'
</script>


<style scoped>
.home {
    display: flex;
    flex-direction: row;
    height: 85vh;
}

.home .left {
    padding-right: 100px;
    border: 3px solid var(--vue-green);
}

.home .right {
}

</style>
